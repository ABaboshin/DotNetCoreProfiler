using Interception.OpenTracing.Prometheus.Extensions;
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

            metric.Tags.AddRange(span.Context.GetBaggageItems().Select(item => new TraceMetric.Types.Tag { Name = item.Key.EscapeTagName(), Value = item.Value }));
            metric.Tags.Add(span.GetTags().Select(item => new TraceMetric.Types.Tag { Name = item.Key.EscapeTagName(), Value = item.Value?.ToString() }));

            _protobufClient.Send(metric);
        }

        private static void SendStatsdFormat(Span span, ILoggerFactory loggerFactory)
        {
            var duration = span.Duration.TotalMilliseconds;
            var metricType = span.OperationName;
            var tags = new Dictionary<string, string>(span.GetTags()?.ToDictionary(t => t.Key, t => t.Value?.ToString()))
            {
                { "traceId", span.Context.TraceId },
                { "spanId", span.Context.SpanId },
                { "parentSpanId", span.Context.ParentSpanId },
                { "service", _serviceConfiguration.Name }
            };

            foreach (var item in span.Context.GetBaggageItems())
            {
                var key = item.Key.EscapeTagName();
                if (!tags.ContainsKey(key))
                {
                    tags.Add(key, item.Value);
                }
            }

            foreach (var item in span.GetTags())
            {
                var key = item.Key.EscapeTagName();
                if (!tags.ContainsKey(key))
                {
                    tags.Add(key, item.Value.ToString());
                }
            }

            tags.Add("type", metricType);
            var startDate = span.StartTimestampUtc.ToUniversalTime().Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds;
            tags.Add("startDate", startDate.ToString());
            var finishDate = span.FinishTimestampUtc?.ToUniversalTime().Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds;
            tags.Add("finishDate", finishDate.ToString());

            loggerFactory.CreateLogger<MetricsSender>()
                .LogDebug("Histogram {name} {duration} {tags}", metricType, duration, string.Join(", ", tags.Select(t => $"{t.Key}={t.Value}")));

            DogStatsd.Histogram("metric", duration, tags:
                new Dictionary<string, string>
            {
                { "type", tags["type"] },
                { "startDate", tags["startDate"] },
                { "finishDate", tags["finishDate"] },
                { "traceId", tags["traceId"] },
                { "spanId", tags["spanId"] },
                { "parentSpanId", tags["parentSpanId"] },
                { "service", tags["service"] },
            }
                .Where(t => t.Value != null).Select(t => $"{t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());
            DogStatsd.Histogram("startDate", startDate, tags:
                new Dictionary<string, string>
            {
                { "traceId", tags["traceId"] },
                { "spanId", tags["spanId"] },
                { "parentSpanId", tags["parentSpanId"] },
            }.Where(t => t.Value != null).Select(t => $"{t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());
            DogStatsd.Histogram("finishDate", finishDate, tags:
                new Dictionary<string, string>
            {
                { "traceId", tags["traceId"] },
                { "spanId", tags["spanId"] },
                { "parentSpanId", tags["parentSpanId"] },
            }.Where(t => t.Value != null).Select(t => $"{t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());

            Console.WriteLine($"finishDate: {finishDate.ToString()}");

            DogStatsd.Histogram(metricType, duration, tags: tags.Where(t => t.Value != null && t.Key != "type").Select(t => $"{ t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());

            if (string.IsNullOrEmpty(span.Context.ParentSpanId))
            {
                DogStatsd.Histogram("metric_info", 1, tags: tags.Where(t => t.Value != null && !new List<string> { "startDate", "finishDate", "spanId", "parentSpanId" }.Contains(t.Key)).Select(t => $"{ t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());
            }
        }
    }
}
