package main

import (
	"bytes"
	"io"
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
	log "github.com/sirupsen/logrus"
)

func listenTCP() error {

	ip, port, err := ExtractIPInfo(os.Getenv("METRIC_PROXY_SERVER__TCP"))

	if err != nil {
		return err
	}

	addr := &net.TCPAddr{
		IP:   ip.IP,
		Port: port,
		Zone: ip.Zone,
	}

	conn, err := net.ListenTCP("tcp", addr)
	if err != nil {
		log.WithFields(log.Fields{
			"error": err,
			"path":  os.Getenv("METRIC_PROXY_SERVER__TCP"),
		}).Fatal("Cannot listen to a TCP socket")
	}

	defer conn.Close()

	log.WithFields(log.Fields{
		"path": os.Getenv("METRIC_PROXY_SERVER__TCP"),
	}).Info("Listen to a TCP socket")

	for {
		c, err := conn.AcceptTCP()

		if err != nil {
			continue
		}

		var buf bytes.Buffer
		io.Copy(&buf, c)

		message := &TraceMetric{}
		err = proto.Unmarshal(buf.Bytes(), message)
		if err != nil {
			log.Debug("Cannot parse a metric, skip")
			continue
		}

		queue.Enqueue(message)
	}
}
