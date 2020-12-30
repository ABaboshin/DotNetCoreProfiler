package main

import (
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
	log "github.com/sirupsen/logrus"
)

func listenUDP() error {

	ip, port, err := ExtractIPInfo(os.Getenv("METRIC_PROXY_SERVER__UDP"))

	if err != nil {
		return err
	}

	addr := &net.UDPAddr{
		IP:   ip.IP,
		Port: port,
		Zone: ip.Zone,
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		log.WithFields(log.Fields{
			"error": err,
			"path":  os.Getenv("METRIC_PROXY_SERVER__UDP"),
		}).Fatal("Cannot listen to a UDP socket")
	}

	defer conn.Close()

	log.Printf("listenUDP [%s]", os.Getenv("METRIC_PROXY_SERVER__UDP"))

	log.WithFields(log.Fields{
		"path": os.Getenv("METRIC_PROXY_SERVER__UDP"),
	}).Info("Listen to a UDP socket")

	buf := make([]byte, 65535)

	for {
		n, _, err := conn.ReadFromUDP(buf)

		message := &TraceMetric{}
		err = proto.Unmarshal(buf[0:n], message)
		if err != nil {
			log.Debug("Cannot parse a metric, skip")
			continue
		}

		queue.Enqueue(message)
	}
}
