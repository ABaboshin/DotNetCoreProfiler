using Interception.Common;
using Interception.Metrics;
using Interception.Metrics.Extensions;
using MassTransit.RabbitMqTransport;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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
                cfg.AddPipeSpecification(new OpenTracingPipeSpecification());

                typedConfigure(cfg);
            };

            return MethodExecutor.ExecuteMethod(null, new object[] { selector, myConfigure }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1[!1]", TargetMethodParametersCount = 1)]
        public static object Consume(object consumer, object content, int mdToken, long moduleVersionPtr)
        {
            var method = consumer.GetType().GetMethod("Consume");

            return Execute(async () => {
                var task = (Task)method.Invoke(consumer, new[] { content });
                await task;
            }, consumer.GetType().FullName);
        }

        private static async Task Execute(Func<Task> action, string consumerName)
        {
            var sw = new Stopwatch();
            sw.Start();
            Exception exception = null;
            try
            {
                await action();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Catched exception");
                exception = ex;
                throw;
            }
            finally
            {
                sw.Stop();

                var tags = new List<string> { $"success:{exception is null}", $"consumer:{consumerName}" };
                if (exception != null)
                {
                    tags.AddRange(exception.GetTags());
                }

                MetricsSender.Histogram("masstransit", (double)sw.ElapsedMilliseconds, tags);
            }
        }
    }
}
