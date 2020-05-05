using Interception.Attributes;
using Interception.Base;
using Interception.MassTransit;
using Interception.Observers;
using Interception.OpenTracing.Prometheus;
using Interception.Quartz;
using Interception.Serilog;
using Interception.Tracing;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using OpenTracing.Util;
using Serilog;
using Serilog.AspNetCore;
using Serilog.Core;
using Serilog.Events;
using Serilog.Formatting.Json;
using System;

namespace Interception.AspNetCore
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            Console.WriteLine($"Configure additional services {_this.GetType().Name} {_parameters[0].GetType().Name} {_parameters[1].GetType().Name}");

            DiagnosticsObserver.ConfigureAndStart();

            var loggerFactory = new SerilogLoggerFactory(CreateLogger(), false);
            var serviceCollection = ((IServiceCollection)_parameters[1]);
            serviceCollection.AddSingleton((ILoggerFactory)loggerFactory);

            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            serviceCollection.Configure<AspNetCoreConfiguration>(configuration.GetSection(AspNetCoreConfiguration.SectionKey));
            serviceCollection.Configure<MassTransitConfiguration>(configuration.GetSection(MassTransitConfiguration.SectionKey));
            serviceCollection.Configure<QuartzConfiguration>(configuration.GetSection(QuartzConfiguration.SectionKey));

            serviceCollection.AddSingleton<IStartupFilter>(_ => new TracingStartupFilter());

            ConfigureMetrics(loggerFactory, serviceCollection);

            return ExecuteInternal(false);
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
