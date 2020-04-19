using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using Serilog;
using Serilog.Core;
using Serilog.Events;
using Serilog.Formatting.Json;
using System;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>()
                .UseSerilog(CreateLogger());
        }

        private static Logger CreateLogger()
        {
            var logLevel = ParseLoggingLevel(Environment.GetEnvironmentVariable("LOG_LEVEL"));

            var logger = new LoggerConfiguration()
                .Enrich.FromLogContext()
                .MinimumLevel.Is(LogEventLevel.Verbose)
                .WriteTo.Console(new JsonFormatter(renderMessage: true), logLevel)
                .CreateLogger();
            return logger;
        }

        private static LogEventLevel ParseLoggingLevel(string logLevelRaw)
        {
            Enum.TryParse(logLevelRaw, out LogEventLevel level);
            return level as LogEventLevel? ?? LogEventLevel.Verbose;
        }
    }
}
