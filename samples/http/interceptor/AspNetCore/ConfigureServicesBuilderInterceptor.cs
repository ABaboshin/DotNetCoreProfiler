using Interception.Attributes;
using Interception.Core;
using Interception.Observers;
using Interception.Tracing.Serilog;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using OpenTracing.Util;
using Serilog.Core;
using Serilog.Events;
using Serilog.Extensions.Logging;
using Serilog.Formatting.Json;
using Serilog;
using System;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept Startup.ConfigureService and inject cache
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "InvokeCore", TargetTypeName = "Microsoft.AspNetCore.Hosting.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            HttpDiagnosticsObserver.ConfigureAndStart();
            EFCoreDiagnosticsObserver.ConfigureAndStart();
            var serviceCollection = ((IServiceCollection)GetParameter(1));

            var logger = CreateLogger();
            var loggerFactory = new SerilogLoggerFactory(logger);
            serviceCollection.AddSingleton((Microsoft.Extensions.Logging.ILoggerFactory)loggerFactory);

            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            serviceCollection.Configure<AspNetCoreConfiguration>(configuration.GetSection(AspNetCoreConfiguration.SectionKey));
            serviceCollection.AddSingleton<IStartupFilter>(_ => new TracingStartupFilter());

            ConfigureMetrics(loggerFactory, serviceCollection);
        }

        private void ConfigureMetrics(Microsoft.Extensions.Logging.ILoggerFactory loggerFactory, IServiceCollection serviceCollection)
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var config = Jaeger.Configuration.FromEnv(loggerFactory);
            var tracer = config.GetTracer();

            GlobalTracer.Register(tracer);

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
