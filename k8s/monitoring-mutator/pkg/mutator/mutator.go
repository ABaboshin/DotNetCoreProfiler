package mutator

import (
	"encoding/json"
	"fmt"
	"log"

	"k8s.io/api/admission/v1beta1"
	admv1beta1 "k8s.io/api/admission/v1beta1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

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

	p := []map[string]string{}
	for i := range pod.Spec.Containers {
		patch := map[string]string{
			"op":    "replace",
			"path":  fmt.Sprintf("/spec/containers/%d/image", i),
			"value": "debian",
		}
		p = append(p, patch)
	}

	resp.Patch, err = json.Marshal(p)

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
