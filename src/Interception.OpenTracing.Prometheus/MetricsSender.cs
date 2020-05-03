﻿using Interception.OpenTracing.Prometheus.Extensions;
using Interception.OpenTracing.Prometheus.protobuf;
using Interception.Tracing;
using Microsoft.Extensions.Logging;
using Statsd.Protobuf.Metrics;
using StatsdClient;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Prometheus
{
    internal class MetricsSender
    {
        internal static ServiceConfiguration _serviceConfiguration;
        internal static StatsdConfiguration _statsdConfiguration;
        internal static ProtobufClient _protobufClient;

        public static void Configure(StatsdConfiguration statsdConfiguration, ServiceConfiguration serviceConfiguration)
        {
            _serviceConfiguration = serviceConfiguration;
            _statsdConfiguration = statsdConfiguration;

            if (!_statsdConfiguration.Protobuf)
            {
                DogStatsd.Configure(new StatsdConfig
                {
                    StatsdServerName = statsdConfiguration.Server,
                    StatsdPort = statsdConfiguration.Port
                });
            }
            else
            {
                _protobufClient = new ProtobufClient(_statsdConfiguration);
            }
        }

        public static void Histogram(Span span, ILoggerFactory loggerFactory)
        {
            if (!_statsdConfiguration.Protobuf)
            {
                SendStatsdFormat(span, loggerFactory);
            }
            else
            {
                try
                {
                    SendProtobufFormat(span, loggerFactory);
                }
                catch (Exception)
                {
                }
            }
        }

        private static void SendProtobufFormat(Span span, ILoggerFactory loggerFactory)
        {
            var metric = new TraceMetric
            {
                Value = span.Duration.TotalMilliseconds,
                Name = "interception",
                Type = "h",
            };

            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "OperationName", Value = span.OperationName });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "TraceId", Value = span.Context.TraceId });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "SpanId", Value = span.Context.SpanId });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "ParentSpanId", Value = span.Context.ParentSpanId });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "Service", Value = _serviceConfiguration.Name });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "StartDate", Value = span.StartTimestampUtc.ToUniversalTime().Subtract(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds.ToString() });
            metric.Tags.Add(new TraceMetric.Types.Tag { Name = "FinishDate", Value = span.FinishTimestampUtc?.ToUniversalTime().Subtract(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds.ToString() });

            metric.Tags.AddRange(span.Context.GetBaggageItems().Select(item => new TraceMetric.Types.Tag { Name = item.Key.Replace("-", "_"), Value = item.Value }));

            _protobufClient.Send(metric);
        }

        private static void SendStatsdFormat(Span span, ILoggerFactory loggerFactory)
        {
            var duration = span.Duration.TotalMilliseconds;
            var metricName = span.OperationName;
            var tags = new Dictionary<string, string>(span.GetTags().ToDictionary(t => t.Key, t => t.Value.ToString()));
            tags.Add("TraceId", span.Context.TraceId);
            tags.Add("SpanId", span.Context.SpanId);
            if (!string.IsNullOrEmpty(span.Context.ParentSpanId))
            {
                tags.Add("ParentSpanId", span.Context.ParentSpanId);
            }

            tags.Add("service", _serviceConfiguration.Name);

            foreach (var item in span.Context.GetBaggageItems())
            {
                var key = item.Key.Replace("-", "_");
                if (!tags.ContainsKey(key))
                {
                    tags.Add(item.Key, item.Value);
                }
            }

            tags.Add("metric", metricName);
            tags.Add("startDate", span.StartTimestampUtc.ToUniversalTime().Subtract(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds.ToString());
            tags.Add("finishDate", span.FinishTimestampUtc?.ToUniversalTime().Subtract(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds.ToString());

            loggerFactory.CreateLogger<MetricsSender>()
                .LogDebug("Histogram {name} {duration} {tags}", metricName, duration, string.Join(", ", tags.Select(t => $"{t.Key}={t.Value}")));

            DogStatsd.Histogram("interception", duration, tags: tags.Select(t => $"{t.Key}:{t.Value.ToString().EscapeTagValue()}").ToArray());
        }
    }
}
