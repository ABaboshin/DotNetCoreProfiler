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

    public static ITracer FromEnv(ILoggerFactory loggerFactory)
    {
      var configuration = new ConfigurationBuilder()
          .AddEnvironmentVariables()
          .Build();

      var statsdConfiguration = configuration.GetSection(StatsdConfiguration.SectionKey).Get<StatsdConfiguration>();
      var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();
      var metricSender = new MetricsSender(serviceConfiguration, statsdConfiguration, loggerFactory);

      return new Tracer(serviceConfiguration, Constants.TraceIdentifier, metricsSender);
    }
  }
}
