using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using SDILReader;
using StackExchange.Redis;
using System;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Test();
            CreateWebHostBuilder(args).Build().Run();
        }

        public static void Test()
        {
            Globals.LoadOpCodes();
            var mi = typeof(Program).GetMethod("Main");

            var mr = new MethodBodyReader(mi);
            string msil = mr.GetBodyCode();
            Console.WriteLine(msil);



            //var body = mi.GetMethodBody();
            //var instructions = body.GetILAsByteArray();
            //Console.WriteLine(instructions.Length);
            //foreach (var i in instructions)
            //{
            //    Console.WriteLine(i);
            //}
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
