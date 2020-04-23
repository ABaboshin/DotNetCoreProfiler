using Interception.OpenTracing.Prometheus.Extensions;
using Interception.Tracing;
using Microsoft.Extensions.Logging;
using StatsdClient;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Prometheus
{
    internal class MetricsSender
    {
        internal static ServiceConfiguration _serviceConfiguration;

        public static void Configure(StatsdConfiguration statsdConfiguration, ServiceConfiguration serviceConfiguration)
        {
            DogStatsd.Configure(new StatsdConfig
            {
                StatsdServerName = statsdConfiguration.Server,
                StatsdPort = statsdConfiguration.Port
            });

            _serviceConfiguration = serviceConfiguration;
        }

        public static void Histogram(Span span, ILoggerFactory loggerFactory)
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
                if (!tags.ContainsKey(item.Key))
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
