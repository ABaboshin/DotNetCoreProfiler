using Interception.OpenTracing.Statsd.Extensions;
using Interception.Tracing;
using Microsoft.Extensions.Logging;
using StatsdClient;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Statsd
{
  internal class MetricsSender : IMetricsSender
  {
    private readonly ServiceConfiguration _serviceConfiguration;
    private readonly StatsdConfiguration _statsdConfiguration;
    private readonly ILoggerFactory _loggerFactory;

    public MetricsSender(ServiceConfiguration serviceConfiguration, StatsdConfiguration statsdConfiguration, ILoggerFactory loggerFactory)
    {
      _serviceConfiguration = serviceConfiguration;
      _statsdConfiguration = statsdConfiguration;
      _loggerFactory = loggerFactory;

      DogStatsd.Configure(new StatsdConfig
      {
        StatsdServerName = statsdConfiguration.Server,
        StatsdPort = statsdConfiguration.Port
      });
    }

    public void Histogram(Span span)
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

      _loggerFactory.CreateLogger<MetricsSender>()
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

      DogStatsd.Histogram(metricType, duration, tags: tags.Where(t => t.Value != null && t.Key != "type").Select(t => $"{ t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());

      if (string.IsNullOrEmpty(span.Context.ParentSpanId))
      {
        DogStatsd.Histogram("metric_info", 1, tags: tags.Where(t => t.Value != null && !new List<string> { "startDate", "finishDate", "spanId", "parentSpanId" }.Contains(t.Key)).Select(t => $"{ t.Key}:{t.Value?.ToString().EscapeTagValue()}").ToArray());
      }
    }
  }
}
