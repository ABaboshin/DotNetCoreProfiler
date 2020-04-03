using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using System;
using System.IO;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {

        public static void Main()
        {
            TestJIT();
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

        private static void TestJIT()
        {
            //Test();
            Test(1, "test", new { x = 1 });
            //new TestC().TestVoid();
            //var res = new TestC().Test();
            //Console.WriteLine(res);
        }

        static void Test()
        {
            Console.WriteLine($"Test empty");
        }

        static void Test(int i1, string s1, object o1) {
            //var ar = new[] { i1, s1, o1 };
            TXX(new[] { i1, s1, o1 }, s1, s1, i1);
            //Console.WriteLine($"Test not empty{i} {s} {o}");
        }

        static void TXX(object[] p, string s, string a, int i)
        {
        }

        public static async Task ATest()
        {
            Console.WriteLine($"Async Test empty");
            await Task.Delay(100);
        }
    }
}
