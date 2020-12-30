package main

import (
	"log"
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
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
		log.Fatal(err)
		os.Exit(1)
	}

	defer conn.Close()

	log.Printf("listenUDP [%s]", os.Getenv("METRIC_PROXY_SERVER__UDP"))

	buf := make([]byte, 65535)

	for {
		n, _, err := conn.ReadFromUDP(buf)

		message := &TraceMetric{}
		err = proto.Unmarshal(buf[0:n], message)
		if err != nil {
			continue
		}

		queue.Enqueue(message)
	}
}
