package main

import (
	"bytes"
	"io"
	"log"
	"net"
	"os"

	"github.com/golang/protobuf/proto"
)

const SockAddr = "/var/tracing-proxy.sock"

func main() {
	if err := os.RemoveAll(SockAddr); err != nil {
		log.Fatal(err)
	}

	l, err := net.ListenUnix("unix", &net.UnixAddr{SockAddr, "unix"})
	if err != nil {
		log.Fatal("listen error:", err)
	}
	defer l.Close()

	log.Printf("Listen [%s] [%s]", SockAddr, "unix")

	for {
		conn, err := l.AcceptUnix()
		if err != nil {
			panic(err)
		}

		var buf bytes.Buffer
		io.Copy(&buf, conn)
		message := &TraceMetric{}
		err = proto.Unmarshal(buf.Bytes(), message)
		if err != nil {
			return
		}

		log.Printf("Got type [%s] value [%f] startDate [%f] finishDate [%f]", *(*message).Type, *(*message).Value, *(*message).StartDate, *(*message).FinishDate)

		conn.Close()
	}
}
