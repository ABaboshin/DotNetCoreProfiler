using MassTransit;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Quartz;
using Quartz.Impl;
using Quartz.Spi;
using SampleApp.Database;
using SampleApp.Database.Entities;
using SampleApp.MessageBus;
using SampleApp.ScheduledJobs;
using System;
using System.Linq;

namespace SampleApp
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
            ConfigureDatabase(serviceCollection);

            serviceCollection.AddMvc();
            ConfigureMessageBus(serviceCollection);

            if (Environment.GetEnvironmentVariable("ENABLE_QUARTZ") == "true")
            {
                ConfigureQuartz(serviceCollection);
            }
        }

        private static void ConfigureQuartz(IServiceCollection serviceCollection)
        {
            serviceCollection.AddSingleton<SampleJob>();
            serviceCollection.AddSingleton<DeadlockJob>();
            serviceCollection.AddSingleton<IJobFactory, JobFactory>();
            serviceCollection.AddSingleton<ISchedulerFactory, StdSchedulerFactory>();
            serviceCollection.AddHostedService<SchedulerService>();
        }

        private void ConfigureMessageBus(IServiceCollection serviceCollection)
        {
            serviceCollection.Configure<RabbitMQConfiguration>(Configuration.GetSection(RabbitMQConfiguration.SectionKey));

            serviceCollection.AddMassTransit(x =>
            {
                x.AddBus(context =>
                {
                    return Bus.Factory.CreateUsingRabbitMq(cfg =>
                    {
                        var loggerFactory = context.GetRequiredService<ILoggerFactory>();
                        cfg.UseExtensionsLogging(loggerFactory);

                        var config = context.GetService<IOptions<RabbitMQConfiguration>>().Value;

                        cfg.Host(new Uri($"rabbitmq://{config.Host}/"), host =>
                        {
                            host.Username(config.User);
                            host.Password(config.Password);
                        });
                        
                        cfg.ReceiveEndpoint("queue-name", ec =>
                        {
                            ec.Consumer(typeof(MyMessageConsumer), t => new MyMessageConsumer(context.GetRequiredService<ILogger<MyMessageConsumer>>()));
                        });
                    });
                });
            });

            serviceCollection.AddHostedService<BusService>();
        }

        private void ConfigureDatabase(IServiceCollection services)
        {
            services.Configure<DatabaseConfiguration>(Configuration.GetSection(DatabaseConfiguration.SectionKey));
            services.AddDbContext<MyDbContext>(options => options.UseSqlServer(Configuration.GetSection(DatabaseConfiguration.SectionKey).Get<DatabaseConfiguration>().ConnectionString));
        }

        public void Configure(IApplicationBuilder app, IHostingEnvironment env)
        {
            app.UseMvc();
            MigrateDatabase(app);
        }

        private static void MigrateDatabase(IApplicationBuilder app)
        {
            using (var scope = app.ApplicationServices.GetService<IServiceScopeFactory>().CreateScope())
            {
                scope.ServiceProvider.GetRequiredService<MyDbContext>().Database.Migrate();

                var context = scope.ServiceProvider.GetRequiredService<MyDbContext>();
                if (!context.MyEntities.Any())
                {
                    context.MyEntities.Add(new MyEntity { Value = "1" });
                    context.MyEntities.Add(new MyEntity { Value = "2" });
                    context.MyEntities.Add(new MyEntity { Value = "3" });
                    context.SaveChanges();
                }
            }
        }
    }
}
