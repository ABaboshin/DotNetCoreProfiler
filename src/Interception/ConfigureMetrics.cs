using Interception.Attributes;
using System;

namespace Interception
{
    [Initialize]
    public class ConfigureMetrics
    {
        public ConfigureMetrics()
        {
            Console.WriteLine("ConfigureMetrics");
        }
    }

}
