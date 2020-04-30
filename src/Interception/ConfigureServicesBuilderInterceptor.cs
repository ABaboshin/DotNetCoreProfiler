﻿using Interception.Common;
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
using System.Collections.Generic;

namespace Interception
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor
    {
        private List<object> _parameters = new List<object>();

        private object _this;

        private int _mdToken;

        private long _moduleVersionPtr;

        public ConfigureServicesBuilderInterceptor()
        {
            Console.WriteLine("ConfigureServicesBuilderInterceptor");
        }

        public object SetThis(object _this)
        {
            Console.WriteLine("SetThis");
            this._this = _this;
            return this;
        }

        public object AddParameter(object value)
        {
            Console.WriteLine("AddParameter");
            _parameters.Add(value);
            return this;
        }

        public object SetMdToken(int mdToken)
        {
            Console.WriteLine("SetMdToken");
            _mdToken = mdToken;
            return this;
        }

        public object SetModuleVersionPtr(long moduleVersionPtr)
        {
            Console.WriteLine("SetModuleVersionPtr");
            _moduleVersionPtr = moduleVersionPtr;
            return this;
        }

        public object Execute()
        {
            Console.WriteLine($"Configure additional services {_this.GetType().Name} {_parameters[0].GetType().Name} {_parameters[1].GetType().Name}");

            DiagnosticsObserver.ConfigureAndStart();

            StackExchangeRedisInterception.Configure();

            var loggerFactory = new SerilogLoggerFactory(CreateLogger(), false);
            var serviceCollection = ((IServiceCollection)_parameters[1]);
            serviceCollection.AddSingleton((ILoggerFactory)loggerFactory);

            ConfigureMetrics(loggerFactory, serviceCollection);

            return MethodExecutor.ExecuteMethod(_this, new object[] { _parameters[0], _parameters[1] }, _mdToken, _moduleVersionPtr, true);
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
