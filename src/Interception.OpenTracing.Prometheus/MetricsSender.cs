using Interception.OpenTracing.Prometheus.Extensions;
using Interception.Tracing;
using StatsdClient;
using System;
using System.Linq;

namespace Interception.OpenTracing.Prometheus
{
    internal static class MetricsSender
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

        public static void Histogram(Span span)
        {
            var duration = span.Duration.TotalMilliseconds;
            var metricName = span.OperationName;
            var tags = span.GetTags();
            tags.Add("TraceId", span.Context.TraceId);
            tags.Add("SpanId", span.Context.SpanId);
            if (!string.IsNullOrEmpty(span.Context.ParentSpanId))
            {
                tags.Add("ParentSpanId", span.Context.ParentSpanId);
            }

            tags.Add("service", _serviceConfiguration.Name);

            foreach (var item in span.Context.GetBaggageItems())
            {
                tags.Add(item.Key, item.Value);
            }

            Console.WriteLine($"Histogram {metricName} {duration} {string.Join(", ", tags.Select(t => $"{t.Key}={t.Value}"))}");

            DogStatsd.Histogram(metricName, duration, tags: tags.Select(t => $"{t.Key}:{t.Value.ToString().EscapeTagValue()}").ToArray());
        }
    }
}
