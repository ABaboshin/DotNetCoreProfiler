using StatsdClient;
using System;

namespace Wrapper
{
    public class Counter
    {
        static object lck = new object();
        static bool configured = false;

        public static void Count(string functionName)
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

            DogStatsd.Counter("function_call", 1, tags: new[] { $"name:{functionName}" });

            Console.WriteLine($"Call to {functionName}");
        }
    }
}
