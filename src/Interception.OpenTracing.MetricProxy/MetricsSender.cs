﻿using Google.Protobuf;
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
    private readonly ServiceConfiguration _serviceConfiguration;
    private readonly ILoggerFactory _loggerFactory;

    public MetricsSender(ServiceConfiguration serviceConfiguration, ILoggerFactory loggerFactory)
    {
      _serviceConfiguration = serviceConfiguration;
      _loggerFactory = loggerFactory;
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

      using (var socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified))
      {
        socket.Connect(new UnixDomainSocketEndPoint("/var/tracing-proxy.sock"));

        var ns = new NetworkStream(socket);
        using (var cod = new CodedOutputStream(ns))
        {
          Console.WriteLine($"start send metric {DateTime.UtcNow.Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds}");
          metric.WriteTo(cod);
          Console.WriteLine($"finish send metric {DateTime.UtcNow.Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds}");
        }
      }
    }
  }
}
