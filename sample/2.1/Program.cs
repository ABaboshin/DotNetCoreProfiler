using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using System.IO;

namespace SampleApp
{
    public class Program
    {
        public static void Main()
        {
            //Test();
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

        //static void Test() { }
    }
}
