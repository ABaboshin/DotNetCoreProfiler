using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {
        public static Task<int> Test()
        {
            return Task.FromResult(3);
        }

        public static void Main(string[] args)
        {
            Type ft = typeof(Func<>);
            var func = ft.Assembly.GetTypes().Where(t => t.Name.StartsWith("Func`")).ToList();
            var testMethod = typeof(Program).GetMethods(System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static).Where(m => m.Name == "Test").First();
            var retType = testMethod.ReturnType;

            CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
