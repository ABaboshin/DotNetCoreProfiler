using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.Net.Sockets;

namespace Interception.OpenTracing.MetricProxy
{
    internal class TcpMetricSender : IUnderlyingMetricSender
    {
        private ILoggerFactory _loggerFactory;
        private readonly TcpClient _tcpClient;

        public TcpMetricSender(ILoggerFactory loggerFactory, string tcp)
        {
            _loggerFactory = loggerFactory;

            var tcpInfo = tcp.Split(":");
            if (tcpInfo.Length != 2)
            {
                throw new ArgumentException(tcp);
            }

            _tcpClient = new TcpClient();
            _tcpClient.Connect(tcpInfo[0], int.Parse(tcpInfo[1]));
            Console.WriteLine($"Connected {_tcpClient.Connected}");
        }

        public bool Send(TraceMetric metric)
        {
            var ns = _tcpClient.GetStream();
            using (var cod = new CodedOutputStream(ns))
            {
                metric.WriteTo(cod);
            }

            return true;
        }
    }
}
