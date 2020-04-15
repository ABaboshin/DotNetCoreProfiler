﻿using Interception.Metrics.Configuration;
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

        public static void Histogram(Action action, MethodBase method = null, string metricName = "", IEnumerable<string> additionalTags = null)
        {
            var sw = new Stopwatch();
            sw.Start();
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

                var tags = new List<string> { $"success:{exception is null}" };
                if (method != null)
                {
                    tags.Add($"name:{method.DeclaringType.Name}.{method.Name}");
                }
                if (exception != null)
                {
                    tags.AddRange(exception.GetTags());
                }

                if (additionalTags != null)
                {
                    tags.AddRange(additionalTags);
                }

                Histogram(metricName ?? "function_call", (double)sw.ElapsedMilliseconds(), tags);
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
