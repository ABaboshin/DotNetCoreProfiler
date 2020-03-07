using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using System.IO;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {
        public static void Main()
        {
            Test();
            Test(1, "test", new { x = 1 });
            new TestC().Test();
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

        static void Test()
        {
            System.Console.WriteLine($"Test empty");
        }

        static void Test(int i, string s, object o) {
            System.Console.WriteLine($"Test not empty{i} {s} {o}");
        }

        public static async Task ATest()
        {
            System.Console.WriteLine($"Async Test empty");
            await Task.Delay(100);
        }
    }
}
