﻿using Interception.Common;
using Interception.Tracing.Extensions;
using MassTransit;
using MassTransit.RabbitMqTransport;
using OpenTracing;
using OpenTracing.Propagation;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    public static class MassTransitInterception
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit.RabbitMqTransport", TargetMethodName = "CreateUsingRabbitMq", TargetTypeName = "MassTransit.BusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
        public static object CreateUsingRabbitMq(object selector, object configure, int mdToken, long moduleVersionPtr)
        {
            var typedConfigure = (Action<IRabbitMqBusFactoryConfigurator>)configure;
            Action<IRabbitMqBusFactoryConfigurator> myConfigure = (IRabbitMqBusFactoryConfigurator cfg) => {
                Console.WriteLine("Masstransit configuration Injected");

                cfg.ConfigurePublish(configurator => configurator.AddPipeSpecification(new OpenTracingPipeSpecification()));

                typedConfigure(cfg);
            };

            return MethodExecutor.ExecuteMethod(null, new object[] { selector, myConfigure }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1[!1]", TargetMethodParametersCount = 1)]
        public static object Consume(object consumer, object context, int mdToken, long moduleVersionPtr)
        {
            var method = consumer.GetType().GetMethod("Consume");

            return Execute(async () => {
                var task = (Task)method.Invoke(consumer, new[] { context });
                await task;
            }, consumer.GetType().FullName, (ConsumeContext)context);
        }

        private static async Task Execute(Func<Task> action, string consumerName, ConsumeContext context)
        {
            var operationName = $"Consuming Message: {context.DestinationAddress}";

            ISpanBuilder spanBuilder;

            try
            {
                var headers = context.Headers.GetAll().ToDictionary(pair => pair.Key, pair => pair.Value.ToString());
                var parentSpanContext = Interception.Tracing.Tracing.Tracer.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(headers));

                spanBuilder = Interception.Tracing.Tracing.Tracer
                    .BuildSpan(operationName)
                    .AsChildOf(parentSpanContext);
            }
            catch (Exception)
            {
                spanBuilder = Interception.Tracing.Tracing.Tracer.BuildSpan(operationName);
            }

            spanBuilder
                .WithTag("consumer", consumerName)
                .WithTag("message-id", context.MessageId?.ToString());

            using (var scope = spanBuilder.StartActive(true))
            {
                try
                {
                    await action();
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }
    }
}
