package main

import (
	"os"

	lfreequeue "github.com/scryner/lfreequeue"
	log "github.com/sirupsen/logrus"
)

var (
	queue = lfreequeue.NewQueue()
)

func main() {

	log.SetFormatter(&log.JSONFormatter{})
	log.SetOutput(os.Stdout)

	switch os.Getenv("METRIC_PROXY_SERVER__LOG_LEVEL") {
	case "debug":
		log.SetLevel(log.DebugLevel)
	case "error":
		log.SetLevel(log.ErrorLevel)
	default:
		log.SetLevel(log.InfoLevel)
	}

	go listenUDS()
	go listenUDP()
	go listenTCP()

	process()
}
