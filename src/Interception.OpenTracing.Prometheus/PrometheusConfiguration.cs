using Interception.Tracing;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using OpenTracing;

namespace Interception.OpenTracing.Prometheus
{
    public class PrometheusConfiguration
    {
        private readonly ILoggerFactory _loggerFactory;
        private readonly ServiceConfiguration _serviceConfiguration;

        public PrometheusConfiguration(ILoggerFactory loggerFactory, ServiceConfiguration serviceConfiguration)
        {
            _loggerFactory = loggerFactory;
            _serviceConfiguration = serviceConfiguration;
        }

        public static PrometheusConfiguration FromEnv(ILoggerFactory loggerFactory)
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var statsdConfiguration = configuration.GetSection(StatsdConfiguration.SectionKey).Get<StatsdConfiguration>();
            var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();

            MetricsSender.Configure(statsdConfiguration, serviceConfiguration);

            return new PrometheusConfiguration(loggerFactory, serviceConfiguration);
        }

        public ITracer GetTracer()
        {
            return new Tracer(_loggerFactory, _serviceConfiguration, "XTraceId");
        }
    }
}
