#if NETCORE21
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args)
        {
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
#endif

#if NETCORE31
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using System.IO;
using System.Reflection;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var assembiles = new[] {
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.dll",
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.Tracing.dll",
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.Cache.dll",
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.DeadlockDetection.dll",
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.MassTransit.dll",
                @"D:\playground\DotNetCoreProfiler\samples\Interception\bin\Debug\netcoreapp3.1\Interception.Quartz.dll",
            };

            //foreach (var item in assembiles)
            //{
            //    Assembly.LoadFrom(item);
            //}

            var host = new WebHostBuilder()
                .UseContentRoot(Directory.GetCurrentDirectory())
                .UseKestrel()
                .UseStartup<Startup>()
                .ConfigureAppConfiguration(cb =>
                {
                    cb.AddEnvironmentVariables();
                })
                .Build();

            host.Run();
        }
    }
}
#endif