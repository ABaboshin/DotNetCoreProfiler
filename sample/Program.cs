using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            int i;
            CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
