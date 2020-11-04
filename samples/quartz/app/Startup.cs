using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Quartz.Impl;
using Quartz.Spi;
using Quartz;
using System.Linq;
using System;
using app.ScheduledJobs;

namespace app
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        public void ConfigureServices(IServiceCollection serviceCollection)
        {
            ConfigureQuartz(serviceCollection);
        }

        private static void ConfigureQuartz(IServiceCollection serviceCollection)
        {
            serviceCollection.AddSingleton<SampleJob>();
            serviceCollection.AddSingleton<IJobFactory, JobFactory>();
            serviceCollection.AddSingleton<ISchedulerFactory, StdSchedulerFactory>();
            serviceCollection.AddHostedService<SchedulerService>();
        }

        public void Configure(IApplicationBuilder app)
        {
        }
    }
}
