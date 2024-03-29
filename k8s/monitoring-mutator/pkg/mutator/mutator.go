package mutator

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"

	"k8s.io/api/admission/v1beta1"
	admv1beta1 "k8s.io/api/admission/v1beta1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/rest"
)

type PatchOperation struct {
	Op    string      `json:"op"`
	Path  string      `json:"path"`
	Value interface{} `json:"value,omitempty"`
}

type Config struct {
	Env            []corev1.EnvVar      `json:"env"`
	Volumes        []corev1.Volume      `json:"volumes"`
	VolumeMounts   []corev1.VolumeMount `json:"volumeMounts"`
	InitContainers []corev1.Container   `json:"initContainers"`
}

var clientset *kubernetes.Clientset

func Init() error {
	config, err := rest.InClusterConfig()
	if err != nil {
		return err
	}

	clientset, err = kubernetes.NewForConfig(config)
	return err
}

func Mutate(body []byte, verbose bool) ([]byte, error) {
	if verbose {
		log.Printf("recv: %s\n", string(body))
	}

	admReview := admv1beta1.AdmissionReview{}
	if err := json.Unmarshal(body, &admReview); err != nil {
		return nil, fmt.Errorf("unmarshaling request failed with %s", err)
	}

	var err error
	var pod *corev1.Pod

	if err := json.Unmarshal(admReview.Request.Object.Raw, &pod); err != nil {
		return nil, fmt.Errorf("unable unmarshal pod json object %v", err)
	}

	responseBody := []byte{}
	resp := admv1beta1.AdmissionResponse{}

	resp.Allowed = true
	resp.UID = admReview.Request.UID
	pT := v1beta1.PatchTypeJSONPatch
	resp.PatchType = &pT

	config, err := getMonitoringConfigurationWithFallback(pod, admReview)

	if err != nil {
		return nil, err
	}

	name, err := getAppName(admReview.Request.Namespace, pod)
	if err != nil {
		return nil, fmt.Errorf("unable get app name %v", err)
	}

	env := config.Env
	env = append(env, corev1.EnvVar{Name: "service__name", Value: name})

	var patches []PatchOperation
	for idx, container := range pod.Spec.Containers {
		patches = append(patches, addEnv(container.Env, env, fmt.Sprintf("/spec/containers/%d/env", idx))...)
	}

	for idx, container := range pod.Spec.Containers {
		patches = append(patches, addVolumeMounts(container.VolumeMounts, config.VolumeMounts, fmt.Sprintf("/spec/containers/%d/volumeMounts", idx))...)
	}

	patches = append(patches, addVolumes(pod.Spec.Volumes, config.Volumes, "/spec/volumes")...)

	patches = append(patches, addInitContainers(pod.Spec.InitContainers, config.InitContainers, "/spec/initContainers")...)

	resp.Patch, err = json.Marshal(patches)

	if err != nil {
		return nil, fmt.Errorf("unable marshal patches %v", err)
	}

	resp.AuditAnnotations = map[string]string{
		"monitoring mutation": "done",
	}

	resp.Result = &metav1.Status{
		Status: "Success",
	}

	admReview.Response = &resp

	responseBody, err = json.Marshal(admReview)
	if err != nil {
		return nil, err
	}

	if verbose {
		log.Printf("resp: %s\n", string(responseBody))
	}

	return responseBody, nil
}

func getMonitoringConfigurationWithFallback(pod *corev1.Pod, admReview admv1beta1.AdmissionReview) (Config, error) {
	if configMap, ok := pod.ObjectMeta.Labels["monitoring-configuration"]; ok {
		customConfig, err := getMonitoringConfiguration(admReview.Request.Namespace, configMap)
		if err != nil {
			log.Println(err)
		} else {
			return customConfig, nil
		}
	}

	return getMonitoringConfiguration(os.Getenv("CURRENT_NAMESPACE"), os.Getenv("DEFAULT_CONFIGURATION"))
}

func getMonitoringConfiguration(namespace string, configmap string) (Config, error) {
	var config Config

	cm, err := clientset.CoreV1().ConfigMaps(namespace).Get(context.TODO(), configmap, metav1.GetOptions{})
	if err != nil {
		log.Printf("1 %v %s %s", err, namespace, configmap)
		return config, err
	}

	err = json.Unmarshal([]byte(cm.Data["config"]), &config)

	if err != nil {
		log.Printf("2 %v %s %s", err, namespace, configmap)
		return config, err
	}

	return config, nil

}

// TODO DaemonSet, Job
func getAppName(namespace string, pod *corev1.Pod) (string, error) {
	if pod.ObjectMeta.OwnerReferences != nil && len(pod.ObjectMeta.OwnerReferences) != 0 {

		if pod.ObjectMeta.OwnerReferences[0].Kind == "StatefulSet" {
			return pod.ObjectMeta.OwnerReferences[0].Name, nil
		}

		if pod.ObjectMeta.OwnerReferences[0].Kind == "ReplicaSet" {
			replica, err := clientset.AppsV1().ReplicaSets(namespace).Get(context.TODO(), pod.ObjectMeta.OwnerReferences[0].Name, metav1.GetOptions{})

			if err != nil {
				return "", fmt.Errorf("unable get replicaset %v", err)
			}

			if replica.ObjectMeta.OwnerReferences != nil && len(replica.ObjectMeta.OwnerReferences) != 0 && replica.ObjectMeta.OwnerReferences[0].Kind == "Deployment" {
				return replica.ObjectMeta.OwnerReferences[0].Name, nil
			}
		}
	}

	return pod.ObjectMeta.Name, nil
}

func addEnv(target, envVars []corev1.EnvVar, basePath string) (patch []PatchOperation) {
	first := len(target) == 0
	var value interface{}
	for _, envVar := range envVars {
		value = envVar
		path := basePath
		if first {
			first = false
			value = []corev1.EnvVar{envVar}
		} else {
			path = path + "/-"
		}
		patch = append(patch, PatchOperation{
			Op:    "add",
			Path:  path,
			Value: value,
		})
	}
	return patch
}

func addVolumes(target, volumes []corev1.Volume, basePath string) (patch []PatchOperation) {
	first := len(target) == 0
	var value interface{}
	for _, volume := range volumes {
		value = volume
		path := basePath
		if first {
			first = false
			value = []corev1.Volume{volume}
		} else {
			path = path + "/-"
		}
		patch = append(patch, PatchOperation{
			Op:    "add",
			Path:  path,
			Value: value,
		})
	}
	return patch
}

func addVolumeMounts(target, volumes []corev1.VolumeMount, basePath string) (patch []PatchOperation) {
	first := len(target) == 0
	var value interface{}
	for _, volume := range volumes {
		value = volume
		path := basePath
		if first {
			first = false
			value = []corev1.VolumeMount{volume}
		} else {
			path = path + "/-"
		}
		patch = append(patch, PatchOperation{
			Op:    "add",
			Path:  path,
			Value: value,
		})
	}
	return patch
}

func addInitContainers(target, containers []corev1.Container, basePath string) (patch []PatchOperation) {
	first := len(target) == 0
	var value interface{}
	for _, container := range containers {
		value = container
		path := basePath
		if first {
			first = false
			value = []corev1.Container{container}
		} else {
			path = path + "/-"
		}
		patch = append(patch, PatchOperation{
			Op:    "add",
			Path:  path,
			Value: value,
		})
	}
	return patch
}
