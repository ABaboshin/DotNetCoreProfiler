using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using SDILReader;
using System;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Test();
            CreateWebHostBuilder(args).Build().Run();
        }

        public Program(string str)
        {

        }

        public static void Test()
        {
            var test = (Program)Activator.CreateInstance(typeof(Program), "xxx");
            var data = new object[] { "test" };
            var assembiles = AppDomain.CurrentDomain.GetAssemblies();
            var types = typeof(Program).Assembly.GetTypes();
            var type = types.Where(t => t.Name == "__InterceptionDllLoaderClass__").FirstOrDefault();
            var method = type.GetMethod("__InterceptionDllLoaderMethod__", BindingFlags.Public | BindingFlags.Static);

            Globals.LoadOpCodes();
            var mi = typeof(Program).GetMethod("Main");
            Dump(typeof(Program).GetMethod("Main"));
            Console.WriteLine();
            Dump(method);
        }

        private static void Dump(MethodInfo mi)
        {
            var mr = new MethodBodyReader(mi);
            string msil = mr.GetBodyCode();
            Console.WriteLine(msil);
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
