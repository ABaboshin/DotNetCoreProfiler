using Google.Protobuf;
using Statsd.Protobuf.Metrics;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Text;

namespace Interception.OpenTracing.Prometheus.protobuf
{
    public class ProtobufClient
    {
        private readonly StatsdConfiguration _statsdConfiguration;

        public ProtobufClient(StatsdConfiguration statsdConfiguration)
        {
            _statsdConfiguration = statsdConfiguration;
        }

        public bool Send(TraceMetric traceMetric)
        {
            var tcp = new TcpClient(_statsdConfiguration.Server, _statsdConfiguration.Port);
            var stream = tcp.GetStream();

            using (var cod = new CodedOutputStream(stream))
            {
                traceMetric.WriteTo(cod);
            }

            stream.Close();

            var ms = new MemoryStream();
            traceMetric.WriteTo(ms);

            Console.WriteLine($"tracemetric {ms.Position}");

            return true;
        }
    }
}
