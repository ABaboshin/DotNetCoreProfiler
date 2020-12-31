using Google.Protobuf;
using Interception.OpenTracing.MetricProxy.Extensions;
using Interception.Tracing;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;

namespace Interception.OpenTracing.MetricProxy
{
    internal class MetricsSender : IMetricsSender
    {
        private readonly MetricProxyConfiguration _metricProxyConfiguration;
        private readonly ServiceConfiguration _serviceConfiguration;
        private readonly ILoggerFactory _loggerFactory;
        private readonly IUnderlyingMetricSender _underlyingMetricSender;

        public MetricsSender(MetricProxyConfiguration metricProxyConfiguration, ServiceConfiguration serviceConfiguration, ILoggerFactory loggerFactory)
        {
            _metricProxyConfiguration = metricProxyConfiguration;
            _serviceConfiguration = serviceConfiguration;
            _loggerFactory = loggerFactory;

            if (!string.IsNullOrEmpty(_metricProxyConfiguration.Udp))
            {
                _underlyingMetricSender = new UdpMetricSender(_loggerFactory, _metricProxyConfiguration.Udp);
            }

            if (!string.IsNullOrEmpty(_metricProxyConfiguration.Uds))
            {
                _underlyingMetricSender = new UdsMetricSender(_loggerFactory, _metricProxyConfiguration.Uds);
            }

            if (!string.IsNullOrEmpty(_metricProxyConfiguration.Tcp))
            {
                _underlyingMetricSender = new TcpMetricSender(_loggerFactory, _metricProxyConfiguration.Tcp);
            }
        }

        public void Histogram(Span span)
        {
            var metric = new TraceMetric();
            metric.Type = span.OperationName;
            metric.Value = span.Duration.TotalMilliseconds;
            metric.TraceId = span.Context.TraceId;
            metric.SpanId = span.Context.SpanId;
            metric.ParentSpanId = span.Context.ParentSpanId;
            metric.Service = _serviceConfiguration.Name;
            metric.StartDate = span.StartTimestampUtc.ToUniversalTime().Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds;
            metric.FinishDate = span.FinishTimestampUtc?.ToUniversalTime().Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds ?? 0;

            metric.Tags.AddRange(span.Context.GetBaggageItems().Select(item => new TraceMetric.Types.Tag { Name = item.Key.EscapeTagName(), Value = item.Value }));
            metric.Tags.Add(span.GetTags().Select(item => new TraceMetric.Types.Tag { Name = item.Key.EscapeTagName(), Value = item.Value?.ToString() }));

            _underlyingMetricSender.Send(metric);
        }
    }
}
