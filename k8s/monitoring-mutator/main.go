package main

import (
	"fmt"
	"html"
	"io/ioutil"
	"log"
	"net/http"
	"time"

	v "github.com/ABaboshin/k8s/monitoring-mutator/pkg/validator"

	ctrl "sigs.k8s.io/controller-runtime"
)

func handleRoot(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "hello %q", html.EscapeString(r.URL.Path))
}

func handleValidate(w http.ResponseWriter, r *http.Request) {
	body, err := ioutil.ReadAll(r.Body)
	defer r.Body.Close()

	if err != nil {
		sendError(err, w)
		return
	}

	validated, err := v.Validate(body, true)
	if err != nil {
		sendError(err, w)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write(validated)
}

func sendError(err error, w http.ResponseWriter) {
	log.Println(err)
	w.WriteHeader(http.StatusInternalServerError)
	fmt.Fprintf(w, "%s", err)
}

func main() {
	// err := os.MkdirAll("/etc/webhook/certs/", 0666)
	// if err != nil {
	// 	log.Panic(err)
	// }

	_ = ctrl.GetConfigOrDie()

	mux := http.NewServeMux()

	mux.HandleFunc("/", handleRoot)
	mux.HandleFunc("/validate", handleValidate)

	s := &http.Server{
		Addr:           ":8443",
		Handler:        mux,
		ReadTimeout:    10 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxHeaderBytes: 1 << 20, // 1048576
	}

	log.Fatal(s.ListenAndServeTLS("/ssl/tls.crt", "/ssl/tls.key"))
}
