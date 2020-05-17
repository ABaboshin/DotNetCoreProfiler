using Interception.Attributes;
using System;

namespace Interception
{
    [Initialize]
    public class ConfigureMetrics
    {
        public ConfigureMetrics()
        {
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

            Console.WriteLine("ConfigureMetrics");
        }

        private System.Reflection.Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            Console.WriteLine($"Try resolve {args.RequestingAssembly.FullName}");
            return null;
        }
    }

}
