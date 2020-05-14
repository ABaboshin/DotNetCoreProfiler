using Interception.Attributes;
using Interception.Cache;
using Interception.Core;
using Interception.MassTransit;
using Interception.Observers;
using Interception.OpenTracing.Prometheus;
using Interception.Quartz;
using Interception.Serilog;
using Interception.Tracing;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.Caching.StackExchangeRedis;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using OpenTracing.Util;
using Serilog;
using Serilog.AspNetCore;
using Serilog.Core;
using Serilog.Events;
using Serilog.Formatting.Json;
using System;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept Startup.ConfigureService and inject
    /// logging
    /// tracing
    /// </summary>
    [StrictIntercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor : BaseInterceptor
    {
        protected override void ExecuteAfter(object result, Exception exception)
        {
        }

        protected override void ExecuteBefore()
        {
            DiagnosticsObserver.ConfigureAndStart();

            var serviceCollection = ((IServiceCollection)GetParameter(1));

            var loggerFactory = new SerilogLoggerFactory(CreateLogger(), false);
            serviceCollection.AddSingleton((ILoggerFactory)loggerFactory);

            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            serviceCollection.Configure<AspNetCoreConfiguration>(configuration.GetSection(AspNetCoreConfiguration.SectionKey));
            serviceCollection.Configure<MassTransitConfiguration>(configuration.GetSection(MassTransitConfiguration.SectionKey));
            serviceCollection.Configure<QuartzConfiguration>(configuration.GetSection(QuartzConfiguration.SectionKey));

            serviceCollection.AddSingleton<IStartupFilter>(_ => new TracingStartupFilter());

            ConfigureMetrics(loggerFactory, serviceCollection);

            ConfigureCache(serviceCollection, configuration);
        }

        private void ConfigureCache(IServiceCollection serviceCollection, IConfigurationRoot configuration)
        {
            serviceCollection.Configure<CacheConfiguration>(configuration.GetSection(CacheConfiguration.SectionKey));
            serviceCollection.AddSingleton<IDistributedCache>(sp => {
                var ccOptions = sp.GetRequiredService<IOptions<CacheConfiguration>>();
                if (ccOptions.Value.Type == "redis")
                {
                    return new RedisCache(Options.Create(new RedisCacheOptions { Configuration = ccOptions.Value.Configuration }));
                }

                return null;
            });
        }

        private void ConfigureMetrics(ILoggerFactory loggerFactory, IServiceCollection serviceCollection)
        {
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

            serviceCollection.AddSingleton(sp => GlobalTracer.Instance);
        }

        private Logger CreateLogger()
        {
            var logLevel = ParseLoggingLevel(Environment.GetEnvironmentVariable("LOG_LEVEL"));

            var logger = new LoggerConfiguration()
                .Enrich.FromLogContext()
                .Enrich.With((ILogEventEnricher)new SerilogEnricher())
                .MinimumLevel.Is(LogEventLevel.Verbose)
                .WriteTo.Console(new JsonFormatter(renderMessage: true), logLevel)
                .CreateLogger();
            return logger;
        }

        private LogEventLevel ParseLoggingLevel(string logLevelRaw)
        {
            Enum.TryParse(logLevelRaw, out LogEventLevel level);
            return level as LogEventLevel? ?? LogEventLevel.Verbose;
        }
    }
}
