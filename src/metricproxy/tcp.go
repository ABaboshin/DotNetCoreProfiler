package main

import (
	"bytes"
	"io"
	"log"
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
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
		log.Fatal(err)
		os.Exit(1)
	}

	defer conn.Close()

	log.Printf("listenTCP [%s]", os.Getenv("METRIC_PROXY_SERVER__TCP"))

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
			continue
		}

		queue.Enqueue(message)
	}
}
