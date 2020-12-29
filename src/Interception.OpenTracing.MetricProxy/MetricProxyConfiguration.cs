using Interception.Tracing;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using OpenTracing;

namespace Interception.OpenTracing.MetricProxy
{
  public class MetricProxyConfiguration
  {
    private readonly ServiceConfiguration _serviceConfiguration;
    private readonly IMetricsSender _metricsSender;

    public MetricProxyConfiguration(ServiceConfiguration serviceConfiguration, IMetricsSender metricsSender)
    {
      _serviceConfiguration = serviceConfiguration;
      _metricsSender = metricsSender;
    }

    public static MetricProxyConfiguration FromEnv(ILoggerFactory loggerFactory)
    {
      var configuration = new ConfigurationBuilder()
          .AddEnvironmentVariables()
          .Build();

      var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();
      var metricSender = new MetricsSender(serviceConfiguration, loggerFactory);

      return new MetricProxyConfiguration(serviceConfiguration, metricSender);
    }

    public ITracer GetTracer()
    {
      return new Tracer(_serviceConfiguration, Constants.TraceIdentifier, _metricsSender);
    }
  }
}
