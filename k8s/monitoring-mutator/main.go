package main

import (
	"crypto/tls"
	"fmt"
	"html"
	"io/ioutil"
	"log"
	"net/http"
	"time"

	m "github.com/ABaboshin/k8s/monitoring-mutator/pkg/mutator"
)

func handleRoot(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "hello %q", html.EscapeString(r.URL.Path))
}

func handleMutate(w http.ResponseWriter, r *http.Request) {
	body, err := ioutil.ReadAll(r.Body)
	defer r.Body.Close()

	if err != nil {
		sendError(err, w)
		return
	}

	mutated, err := m.Mutate(body, true)
	if err != nil {
		sendError(err, w)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write(mutated)
}

func sendError(err error, w http.ResponseWriter) {
	log.Println(err)
	w.WriteHeader(http.StatusInternalServerError)
	fmt.Fprintf(w, "%s", err)
}

func main() {
	mux := http.NewServeMux()

	mux.HandleFunc("/", handleRoot)
	mux.HandleFunc("/mutate", handleMutate)

	pair, err := tls.LoadX509KeyPair("/ssl/tls.crt", "/ssl/tls.key")

	if err != nil {
		log.Panicf("Error loading keys %v", err)
	}

	s := &http.Server{
		Addr:           ":8443",
		Handler:        mux,
		ReadTimeout:    10 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxHeaderBytes: 1 << 20,
		TLSConfig:      &tls.Config{Certificates: []tls.Certificate{pair}},
	}

	err = m.Init()
	if err != nil {
		log.Panicf("Error init %v", err)
	}

	log.Fatal(s.ListenAndServeTLS("", ""))
}
