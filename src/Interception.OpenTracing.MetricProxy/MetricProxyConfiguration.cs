using Interception.Tracing;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using OpenTracing;

namespace Interception.OpenTracing.MetricProxy
{
    public class MetricProxyConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "metric_proxy";

        /// <summary>
        /// metric proxy server udp address
        /// </summary>
        public string Udp { get; set; }

        /// <summary>
        /// metric proxy server uds address
        /// </summary>
        public string Uds { get; set; }

        /// <summary>
        /// metric proxy server tcp address
        /// </summary>
        public string Tcp { get; set; }

        public static ITracer FromEnv(ILoggerFactory loggerFactory)
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var metricProxyConfiguration = configuration.GetSection(MetricProxyConfiguration.SectionKey).Get<MetricProxyConfiguration>();
            var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();
            var metricSender = new MetricsSender(metricProxyConfiguration, serviceConfiguration, loggerFactory);

            return new Tracer(serviceConfiguration, Constants.TraceIdentifier, metricSender);
        }
    }
}
