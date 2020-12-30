using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace Interception.OpenTracing.MetricProxy
{
    internal class UdpMetricSender : IUnderlyingMetricSender
    {
        private ILoggerFactory _loggerFactory;
        private readonly Socket _socket;
        private readonly IPEndPoint _endPoint;

        public UdpMetricSender(ILoggerFactory loggerFactory, string udp)
        {
            _loggerFactory = loggerFactory;

            var udpInfo = udp.Split(":");
            if (udpInfo.Length != 2)
            {
                throw new ArgumentException(udp);
            }

            _endPoint = new IPEndPoint(Util.GetIpv4Address(udpInfo[0]), int.Parse(udpInfo[1]));

            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

            try
            {
                // When closing, wait 2 seconds to send data.
                _socket.LingerState = new LingerOption(true, 2);
            }
            catch (SocketException e) when (e.SocketErrorCode == SocketError.ProtocolOption)
            {
                // It is not supported on Windows for Dgram with UDP.
            }
        }

        public void Send(TraceMetric metric)
        {
            Console.WriteLine($"start send metric {DateTime.UtcNow.Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds}");
            
            var ms = new MemoryStream();
            using (var cod = new CodedOutputStream(ms))
            {
                metric.WriteTo(cod);
                ms.Flush();
            }

            var ar = ms.ToArray();
            _socket.SendTo(ar, 0, ar.Length, SocketFlags.None, _endPoint);

            Console.WriteLine($"finish send metric {DateTime.UtcNow.Subtract(new DateTime(2020, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds}");
        }
    }
}