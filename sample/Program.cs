using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {
        public static async Task T1() {
            throw new System.Exception();
        }

        public static async Task<T> T2<T>()
        {
            throw new System.Exception();
        }

        public static object T3()
        {
            throw new System.Exception();
        }

        public static void Main(string[] args)
        {
            var methods = typeof(Program).GetMethods(System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            var t1 = methods.Where(m => m.Name == "T1").FirstOrDefault();
            var t2 = methods.Where(m => m.Name == "T2").FirstOrDefault();
            var t3 = methods.Where(m => m.Name == "T3").FirstOrDefault();

            Analyze(t1);
            Analyze(t2);
            Analyze(t3);

            CreateWebHostBuilder(args).Build().Run();
        }

        public static void Analyze(MethodInfo method)
        {
            var ret = method.ReturnType;
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
