using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using System;
using System.IO;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            foreach (var item in AppDomain.CurrentDomain.GetAssemblies())
            {
                Console.WriteLine(item.FullName);
            }

            //AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

            CreateWebHostBuilder(args).Build().Run();
        }

        private static System.Reflection.Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            var test = args.RequestingAssembly.FullName;
            return null;
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
