package main

import (
	"log"

	lfreequeue "github.com/scryner/lfreequeue"
)

var (
	queue = lfreequeue.NewQueue()
)

func main() {

	watchIterator := queue.NewWatchIterator()
	iter := watchIterator.Iter()
	defer watchIterator.Close()

	go listenUDS()
	go listenUDP()

	for {
		select {
		case v := <-iter:
			val := v.(*TraceMetric)
			log.Printf("Got type [%s] value [%f] startDate [%f] finishDate [%f]", *val.Type, *val.Value, *val.StartDate, *val.FinishDate)
		}
	}
}
