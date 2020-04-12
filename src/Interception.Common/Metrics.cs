using StatsdClient;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;

namespace Interception.Common
{
    public static class Metrics
    {
        static object lck = new object();
        static bool configured = false;

        public static void Configure()
        {
            lock (lck)
            {
                if (!configured)
                {
                    DogStatsd.Configure(new StatsdConfig
                    {
                        StatsdServerName = "statsd",
                        StatsdPort = 9125
                    });

                    configured = true;
                }
            }
        }

        public static void Histogram(Action action, MethodBase method)
        {
            Configure();
            var sw = new Stopwatch();
            try
            {
                action();
                sw.Stop();
                DogStatsd.Histogram("function_call", (double)sw.ElapsedMilliseconds(), tags: new[] { $"name:{method.DeclaringType.Name}.{method.Name}", "success:true" });
            }
            catch (Exception ex)
            {
                var tags = new List<string> { $"name:{method.DeclaringType.Name}.{method.Name}", "success:false" };
                tags.AddRange(ex.GetTags());
                DogStatsd.Histogram("function_call", (double)sw.ElapsedMilliseconds(), tags: tags.ToArray());
                throw;
            }
        }

        public static void Histogram(string name, double duration, string[] tags)
        {
            Console.WriteLine($"Histogram {name} {duration} {tags}");
            Configure();

            DogStatsd.Histogram(name, duration, tags: tags);
        }
    }
}
