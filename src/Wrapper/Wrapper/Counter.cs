using StatsdClient;
using System;

namespace Wrapper
{
    public class Counter
    {
        public static void Count(string functionName)
        {
            DogStatsd.Counter("function_call", 1, tags: new[] { $"name:{functionName}" });

            Console.WriteLine($"Call to {functionName}");
        }
    }
}
