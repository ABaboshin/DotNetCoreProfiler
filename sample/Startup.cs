//using Interception.MassTransit;
//using Interception.Observers;
using MassTransit;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
//using OpenTracing;
//using OpenTracing.Util;
using SampleApp.Database;
using SampleApp.Database.Entities;
using SampleApp.MessageBus;
using SampleApp.Redis;
using StackExchange.Redis;
using System;
using System.Diagnostics;
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
            //DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(new Interception.Observers.Configuration.HttpConfiguration { Enabled = true }));

            //var loggerFactory = new LoggerFactory();
            //var config = Jaeger.Configuration.FromEnv(loggerFactory);
            //var tracer = config.GetTracer();
            //GlobalTracer.Register(tracer);

            ConfigureDatabase(serviceCollection);

            serviceCollection.AddMvc();
            ConfigureMessageBus(serviceCollection);
            ConfigureRedis(serviceCollection);

            //services.AddOpenTracing();
        }

        private void ConfigureRedis(IServiceCollection services)
        {
            services.Configure<RedisConfiguration>(Configuration.GetSection(RedisConfiguration.SectionKey));
            services.AddScoped<ConnectionMultiplexer>(sp =>
            {
                var config = sp.GetService<IOptions<RedisConfiguration>>().Value;
                return ConnectionMultiplexer.Connect(config.ConnectionString);
            });
            services.AddHostedService<RedisSubscriber>();
        }

        private void ConfigureMessageBus(IServiceCollection services)
        {
            services.Configure<RabbitMQConfiguration>(Configuration.GetSection(RabbitMQConfiguration.SectionKey));

            services.AddMassTransit(x =>
            {
                x.AddBus(context =>
                {
                    return Bus.Factory.CreateUsingRabbitMq(cfg =>
                    {
                        var loggerFactory = context.GetRequiredService<ILoggerFactory>();
                        cfg.UseExtensionsLogging(loggerFactory);

                        //cfg.ConfigurePublish(configurator => configurator.AddPipeSpecification(new OpenTracingPipeSpecification()));
                        //cfg.AddPipeSpecification(new OpenTracingPipeSpecification());

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

            services.AddHostedService<BusService>();
        }

        private void ConfigureDatabase(IServiceCollection services)
        {
            services.Configure<DatabaseConfiguration>(Configuration.GetSection(DatabaseConfiguration.SectionKey));
            services.AddDbContext<MyDbContext>(options => options.UseSqlServer(Configuration.GetSection(DatabaseConfiguration.SectionKey).Get<DatabaseConfiguration>().ConnectionString));
        }

        public void Configure(IApplicationBuilder app, IHostingEnvironment env)
        {
            //if (env.IsDevelopment())
            //{
            //    app.UseDeveloperExceptionPage();
            //}

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
