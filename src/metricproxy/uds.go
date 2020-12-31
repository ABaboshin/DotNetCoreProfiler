package main

import (
	"bytes"
	"io"
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
	log "github.com/sirupsen/logrus"
)

func listenUDS() error {

	sockAddr := os.Getenv("METRIC_PROXY_SERVER__UDS")

	if sockAddr == "" {
		return nil
	}

	if err := os.RemoveAll(sockAddr); err != nil {
		log.WithFields(log.Fields{
			"error": err,
			"path":  sockAddr,
		}).Fatal("Cannot remove an unix domain socket")
	}

	l, err := net.ListenUnix("unix", &net.UnixAddr{sockAddr, "unix"})
	if err != nil {
		log.WithFields(log.Fields{
			"error": err,
			"path":  sockAddr,
		}).Fatal("Cannot listen to an unix domain socket")
	}
	defer l.Close()

	log.WithFields(log.Fields{
		"path": sockAddr,
	}).Info("Listen to an unix domain socket")

	for {
		conn, err := l.AcceptUnix()
		if err != nil {
			continue
		}

		var buf bytes.Buffer
		io.Copy(&buf, conn)
		message := &TraceMetric{}
		err = proto.Unmarshal(buf.Bytes(), message)
		if err != nil {
			log.Debug("Cannot parse a metric, skip")
			continue
		}

		queue.Enqueue(message)

		conn.Close()
	}
}
