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
