package main

import (
	"fmt"
	"os"
	"strings"

	"github.com/DataDog/datadog-go/statsd"
	log "github.com/sirupsen/logrus"
)

func process() {

	watchIterator := queue.NewWatchIterator()
	iter := watchIterator.Iter()
	defer watchIterator.Close()

	statsd, err := statsd.New(os.Getenv("METRIC_PROXY_SERVER__STATSD"))
	if err != nil {
		log.WithFields(log.Fields{
			"error": err,
			"path":  os.Getenv("METRIC_PROXY_SERVER__STATSD"),
		}).Fatal("Cannot connect to statsd exporter")
	}

	log.WithFields(log.Fields{
		"path": os.Getenv("METRIC_PROXY_SERVER__STATSD"),
	}).Info("Connected to statsd exporter")

	for {
		select {
		case v := <-iter:
			val := v.(*TraceMetric)

			log.WithFields(log.Fields{
				"type":       *val.Type,
				"value":      *val.Value,
				"startDate":  *val.StartDate,
				"finishDate": *val.FinishDate,
				"traceId":    *val.TraceId,
			}).Debug("Process metric")

			err = statsd.Histogram(
				"startDate",
				*val.StartDate,
				[]string{
					fmt.Sprintf("traceId:%s", *val.TraceId),
					fmt.Sprintf("spanId:%s", *val.SpanId),
					fmt.Sprintf("parentSpanId:%s", *val.ParentSpanId),
				},
				1)

			statsd.Histogram(
				"finishDate",
				*val.FinishDate,
				[]string{
					fmt.Sprintf("traceId:%s", *val.TraceId),
					fmt.Sprintf("spanId:%s", *val.SpanId),
					fmt.Sprintf("parentSpanId:%s", *val.ParentSpanId),
				},
				1)

			statsd.Histogram(
				"metric",
				*val.Value,
				[]string{
					fmt.Sprintf("traceId:%s", *val.TraceId),
					fmt.Sprintf("spanId:%s", *val.SpanId),
					fmt.Sprintf("parentSpanId:%s", *val.ParentSpanId),
					fmt.Sprintf("type:%s", *val.Type),
					fmt.Sprintf("startDate:%f", *val.StartDate),
					fmt.Sprintf("finishDate:%f", *val.FinishDate),
					fmt.Sprintf("service:%s", *val.Service),
				},
				1)

			var tags []string

			for _, t := range val.Tags {
				tags = append(tags, fmt.Sprintf("%s:%s", *t.Name, *t.Value))
			}

			tags = append(tags, fmt.Sprintf("traceId:%s", *val.TraceId))
			tags = append(tags, fmt.Sprintf("service:%s", *val.Service))
			tags = append(tags, fmt.Sprintf("type:%s", *val.Type))

			if *val.ParentSpanId == "" {
				statsd.Histogram(
					"metric_info",
					1,
					tags,
					1)
			}

			statsd.Histogram(
				"metric_stat",
				*val.Value,
				filter(tags, func(s string) bool { return !strings.HasPrefix(s, "type:") && !strings.HasPrefix(s, "traceId:") }),
				1)

			tags = append(tags, fmt.Sprintf("startDate:%f", *val.StartDate))
			tags = append(tags, fmt.Sprintf("finishDate:%f", *val.FinishDate))
			tags = append(tags, fmt.Sprintf("parentSpanId:%s", *val.ParentSpanId))
			tags = append(tags, fmt.Sprintf("spanId:%s", *val.SpanId))

			statsd.Histogram(
				*val.Type,
				*val.Value,
				filter(tags, func(s string) bool { return !strings.HasPrefix(s, "type:") }),
				1)
		}
	}
}
