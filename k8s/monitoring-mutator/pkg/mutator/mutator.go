package mutator

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"path/filepath"

	yaml "gopkg.in/yaml.v2"
	"k8s.io/api/admission/v1beta1"
	admv1beta1 "k8s.io/api/admission/v1beta1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

type PatchOperation struct {
	Op    string      `json:"op"`
	Path  string      `json:"path"`
	Value interface{} `json:"value,omitempty"`
}

type Config struct {
	Env []corev1.EnvVar `yaml:"env"`
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

	log.Printf("recv namespace: %s\n", admReview.Request.Namespace)

	resp.Allowed = true
	resp.UID = admReview.Request.UID
	pT := v1beta1.PatchTypeJSONPatch
	resp.PatchType = &pT

	filename, _ := filepath.Abs("/config.yml")
	yamlFile, err := ioutil.ReadFile(filename)

	if err != nil {
		return nil, fmt.Errorf("unable marshal patches %v", err)
	}

	log.Println(yamlFile)

	var config Config

	err = yaml.Unmarshal(yamlFile, &config)

	if err != nil {
		return nil, fmt.Errorf("unable marshal patches %v", err)
	}

	log.Println(config)

	var patches []PatchOperation
	for idx, container := range pod.Spec.Containers {
		patches = append(patches, addEnv(container.Env, config.Env, fmt.Sprintf("/spec/containers/%d/env", idx))...)
	}

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
		log.Printf("resp: %s\n", string(responseBody)) // untested section
	}

	return responseBody, nil
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
