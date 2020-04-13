using Interception.Common;
using Interception.Metrics;
using Interception.Metrics.Extensions;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Interception
{
    public static class MassTransitInterception
    {
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
