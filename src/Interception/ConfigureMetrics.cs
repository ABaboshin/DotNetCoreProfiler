using Interception.Common;
using Interception.Metrics;
using Interception.Metrics.Configuration;
using Microsoft.Extensions.Configuration;
using System;

namespace Interception
{
    [Initialize]
    public class ConfigureMetrics
    {
        public ConfigureMetrics()
        {
            Console.WriteLine("Initialize");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var statsdConfiguration = configuration.GetSection(StatsdConfiguration.SectionKey).Get<StatsdConfiguration>();
            var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();

            MetricsSender.Configure(statsdConfiguration, serviceConfiguration);
        }
    }
}
