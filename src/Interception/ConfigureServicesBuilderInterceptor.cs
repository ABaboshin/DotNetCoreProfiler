using Interception.Common;
using Interception.Observers;
using Interception.OpenTracing.Prometheus;
using Interception.Serilog;
using Interception.StackExchangeRedis;
using Interception.Tracing;
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

namespace Interception
{
    public static class ConfigureServicesBuilderInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
        public static object Invoke(object builder, object instance, object services, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"Configure additional services {builder.GetType().Name} {instance.GetType().Name} {services.GetType().Name}");

            DiagnosticsObserver.ConfigureAndStart();

            StackExchangeRedisInterception.Configure();

            var loggerFactory = new SerilogLoggerFactory(CreateLogger(), false);
            var serviceCollection = ((IServiceCollection)services);
            serviceCollection.AddSingleton((ILoggerFactory)loggerFactory);

            ConfigureMetrics(loggerFactory, serviceCollection);

            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);
        }

        private static void ConfigureMetrics(ILoggerFactory loggerFactory, IServiceCollection serviceCollection)
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

        private static Logger CreateLogger()
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

        private static LogEventLevel ParseLoggingLevel(string logLevelRaw)
        {
            Enum.TryParse(logLevelRaw, out LogEventLevel level);
            return level as LogEventLevel? ?? LogEventLevel.Verbose;
        }
    }
}
