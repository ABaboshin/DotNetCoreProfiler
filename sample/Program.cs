using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using StackExchange.Redis;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var type = typeof(ChannelMessage);
            var ctors = type.GetConstructors(System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            var ctor = ctors[0];
            var rv = (RedisValue)"test";
            var cm = ctor.Invoke(new object[] { null, new RedisChannel("test", RedisChannel.PatternMode.Auto), rv });

            CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
