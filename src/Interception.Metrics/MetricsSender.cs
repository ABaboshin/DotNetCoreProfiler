using Interception.Metrics.Configuration;
using Interception.Metrics.Extensions;
using StatsdClient;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace Interception.Metrics
{
    public static class MetricsSender
    {
        private static ServiceConfiguration _serviceConfiguration;

        public static void Configure(StatsdConfiguration statsdConfiguration, ServiceConfiguration serviceConfiguration)
        {
            DogStatsd.Configure(new StatsdConfig
            {
                StatsdServerName = statsdConfiguration.Server,
                StatsdPort = statsdConfiguration.Port
            });

            _serviceConfiguration = serviceConfiguration;
        }

        public static void Histogram(Action action, MethodBase method)
        {
            var sw = new Stopwatch();
            Exception exception = null;
            try
            {
                action();
            }
            catch (Exception ex)
            {
                exception = ex;
                throw;
            }
            finally
            {
                sw.Stop();

                var tags = new List<string> { $"name:{method.DeclaringType.Name}.{method.Name}", $"success:{exception != null}" };
                if (exception != null)
                {
                    tags.AddRange(exception.GetTags());
                }

                Histogram("function_call", (double)sw.ElapsedMilliseconds(), tags);
            }
        }

        public static void Histogram(string name, double duration, List<string> tags)
        {
            tags.Add($"service:{_serviceConfiguration.Name}");

            Console.WriteLine($"Histogram {name} {duration} {tags}");

            DogStatsd.Histogram(name, duration, tags: tags.ToArray());
        }
    }
}
