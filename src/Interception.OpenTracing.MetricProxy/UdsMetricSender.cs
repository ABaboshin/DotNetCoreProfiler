using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.IO;
using System.Net.Sockets;

namespace Interception.OpenTracing.MetricProxy
{
    internal class UdsMetricSender : IUnderlyingMetricSender
    {
        private ILogger<UdsMetricSender> _logger;
        private string _uds;

        public UdsMetricSender(ILoggerFactory loggerFactory, string uds)
        {
            _logger = loggerFactory.CreateLogger<UdsMetricSender>();
            _uds = uds;
        }

        public bool Send(TraceMetric metric)
        {
            if (!File.Exists(_uds)) {
                _logger.LogError("Metric proxy socket not found");
                return false;
            }

            using (var socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified))
            {
                try
                {
                    socket.Connect(new UnixDomainSocketEndPoint(_uds));
                }
                catch (Exception e)
                {
                    _logger.LogError(e, "Cannot connect to a metric proxy");
                    return false;
                }

                var ns = new NetworkStream(socket);
                using (var cod = new CodedOutputStream(ns))
                {
                    try
                    {
                        metric.WriteTo(cod);
                    }
                    catch (Exception e)
                    {
                        _logger.LogError(e, "Cannot send to a metric proxy");
                        return false;
                    }
                }

                return true;
            }
        }
    }
}
