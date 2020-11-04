using MassTransit;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System.Linq;
using System;
using app.MessageBus;

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
            serviceCollection
                .AddMvc(options => { options.EnableEndpointRouting = false; });

            ConfigureMessageBus(serviceCollection);
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
                        // cfg.UseExtensionsLogging(loggerFactory);

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

        public void Configure(IApplicationBuilder app)
        {
            app.UseMvc();
        }
    }
}
