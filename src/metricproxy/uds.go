package main

import (
	"bytes"
	"io"
	"log"
	"net"
	"os"

	proto "github.com/golang/protobuf/proto"
)

func listenUDS() error {

	sockAddr := os.Getenv("METRIC_PROXY_SERVER__UDS")

	if sockAddr == "" {
		return nil
	}

	if err := os.RemoveAll(sockAddr); err != nil {
		log.Fatal(err)
	}

	l, err := net.ListenUnix("unix", &net.UnixAddr{sockAddr, "unix"})
	if err != nil {
		log.Fatal("listen error:", err)
	}
	defer l.Close()

	log.Printf("listenUDS [%s]", sockAddr)

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
			continue
		}

		queue.Enqueue(message)

		conn.Close()
	}
}
