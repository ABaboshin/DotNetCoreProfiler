using Interception.Common;
using Interception.OpenTracing.Prometheus;
using Interception.Tracing;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using OpenTracing.Util;
using System;

namespace Interception
{
    [Initialize]
    public class ConfigureMetrics
    {
        public ConfigureMetrics()
        {
            Console.WriteLine("ConfigureMetrics");
            var loggerFactory = new LoggerFactory();

            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var tracingConfiguration = configuration.GetSection(TracingConfiguration.SectionKey).Get<TracingConfiguration>();

            if (tracingConfiguration.Collector.ToLower() == "jaeger")
            {
                var config = Jaeger.Configuration.FromEnv(loggerFactory);
                var tracer = config.GetTracer();

                GlobalTracer.Register(tracer);
            }
            else
            {
                var config = PrometheusConfiguration.FromEnv(loggerFactory);
                var tracer = config.GetTracer();

                GlobalTracer.Register(tracer);
            }
        }
    }
}
