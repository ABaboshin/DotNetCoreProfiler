using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.Net.Sockets;

namespace Interception.OpenTracing.MetricProxy
{
    internal class UdsMetricSender : IUnderlyingMetricSender
    {
        private ILoggerFactory _loggerFactory;
        private string _uds;

        public UdsMetricSender(ILoggerFactory loggerFactory, string uds)
        {
            _loggerFactory = loggerFactory;
            _uds = uds;
        }

        public void Send(TraceMetric metric)
        {
            using (var socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified))
            {
                socket.Connect(new UnixDomainSocketEndPoint(_uds));

                var ns = new NetworkStream(socket);
                using (var cod = new CodedOutputStream(ns))
                {
                    metric.WriteTo(cod);
                }
            }
        }
    }
}