using Interception.Tracing;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using OpenTracing;

namespace Interception.OpenTracing.Statsd
{
  public class StatsdConfiguration
  {

    /// <summary>
    /// Section name in configuration
    /// </summary>
    public static readonly string SectionKey = "statsd";

    /// <summary>
    /// statsd server
    /// </summary>
    public string Server { get; set; }

    /// <summary>
    /// statsd port
    /// </summary>
    public int Port { get; set; }

    private readonly ServiceConfiguration _serviceConfiguration;
    private readonly IMetricsSender _metricsSender;

    public StatsdConfiguration(ServiceConfiguration serviceConfiguration, IMetricsSender metricsSender)
    {
      _serviceConfiguration = serviceConfiguration;
      _metricsSender = metricsSender;
    }

    public static StatsdConfiguration FromEnv(ILoggerFactory loggerFactory)
    {
      var configuration = new ConfigurationBuilder()
          .AddEnvironmentVariables()
          .Build();

      var statsdConfiguration = configuration.GetSection(StatsdConfiguration.SectionKey).Get<StatsdConfiguration>();
      var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();
      var metricSender = new MetricsSender(serviceConfiguration, statsdConfiguration, loggerFactory);

      return new StatsdConfiguration(serviceConfiguration, metricSender);
    }

    public ITracer GetTracer()
    {
      return new Tracer(_serviceConfiguration, Constants.TraceIdentifier, _metricsSender);
    }
  }
}
