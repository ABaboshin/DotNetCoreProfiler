using StatsdClient;
using System;

public class Configuration
{
    static object lck = new object();
    static bool configured = false;

    public static void Configure()
    {
        lock (lck)
        {
            if (!configured)
            {
                Console.WriteLine("Configuration.Configure");
                DogStatsd.Configure(new StatsdConfig
                {
                    StatsdServerName = "statsd",
                    StatsdPort = 9125
                });

                configured = true;
            }
        }
    }

    public Configuration()
    {
        Console.WriteLine("Configuration");
        Configure();
    }

    public static Configuration instance = new Configuration();
}
