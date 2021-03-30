package validator

import (
	"encoding/json"
	"fmt"
	"log"

	admv1beta1 "k8s.io/api/admission/v1beta1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

func Validate(body []byte, verbose bool) ([]byte, error) {
	if verbose {
		log.Printf("recv: %s\n", string(body))
	}

	admReview := admv1beta1.AdmissionReview{}
	if err := json.Unmarshal(body, &admReview); err != nil {
		return nil, fmt.Errorf("unmarshaling request failed with %s", err)
	}

	var err error

	responseBody := []byte{}
	resp := admv1beta1.AdmissionResponse{}

	log.Printf("recv namespace: %s\n", admReview.Request.Namespace)

	resp.Allowed = true
	resp.UID = admReview.Request.UID

	resp.AuditAnnotations = map[string]string{
		"ingress validation": "done",
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
