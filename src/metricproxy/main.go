package main

import (
	lfreequeue "github.com/scryner/lfreequeue"
)

var (
	queue = lfreequeue.NewQueue()
)

func main() {

	go listenUDS()
	go listenUDP()
	go listenTCP()

	process()
}
