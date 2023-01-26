using System;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Interception.AspNetCore;
using Interception.Attributes;
using Interception.Observers;
using Interception.Observers.Samplers;
using Interception.Tracing.Serilog;
using OpenTracing;
using OpenTracing.Mock;
using OpenTracing.Util;
using Serilog;
using Serilog.Core;
using Serilog.Extensions.Logging;
using System.Runtime.CompilerServices;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept Startup.ConfigureService and inject cache
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "InvokeCore", TargetTypeName = "Microsoft.AspNetCore.Hosting.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void After<TResult>(TResult result, Exception ex)
        {
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Before<TType, T1, T2>(TType instance, ref T1 a1, ref T2 serviceCollection) where T2 : IServiceCollection
        {
            HttpDiagnosticsObserver.ConfigureAndStart();
            EFCoreDiagnosticsObserver.ConfigureAndStart();

            //CPUSampler.ConfigureAndStart();
            //MemSampler.ConfigureAndStart();
            //GCSampler.ConfigureAndStart();

            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var logger = CreateLogger(configuration);
            var loggerFactory = new SerilogLoggerFactory(logger);
            serviceCollection.AddSingleton((Microsoft.Extensions.Logging.ILoggerFactory)loggerFactory);

            serviceCollection.Configure<AspNetCoreConfiguration>(configuration.GetSection(AspNetCoreConfiguration.SectionKey));
            serviceCollection.AddSingleton<IStartupFilter>(_ => new TracingStartupFilter());

            ConfigureMetrics(loggerFactory, serviceCollection);
        }

        private static void ConfigureMetrics(Microsoft.Extensions.Logging.ILoggerFactory loggerFactory, IServiceCollection serviceCollection)
        {
          ITracer tracer = null;
          try
          {
            var configuration = new ConfigurationBuilder()
              .AddEnvironmentVariables()
              .Build();

            var config = Jaeger.Configuration.FromEnv(loggerFactory);

            tracer = config.GetTracer();

            // initialize dogstatsd client
            // var statsdConfiguration = Interception.OpenTracing.Statsd.StatsdConfiguration.FromEnv(loggerFactory);
          }
          catch (System.Exception)
          {
              tracer = new MockTracer();
          }

          GlobalTracer.Register(tracer);

          serviceCollection.AddSingleton(sp => GlobalTracer.Instance);
        }

        private static Logger CreateLogger(IConfiguration configuration)
        {
            var logger =
                new LoggerConfiguration()
                    .ReadFrom.Configuration(configuration)
                    .Enrich.With((ILogEventEnricher)new SerilogEnricher())
                    .CreateLogger();

            return logger;
        }
    }
}
