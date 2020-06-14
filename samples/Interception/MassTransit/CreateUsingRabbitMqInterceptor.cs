using Interception.Attributes;
using Interception.Core;
using MassTransit.RabbitMqTransport;
using Microsoft.Extensions.Configuration;
using System;

namespace Interception.MassTransit
{
    /// <summary>
    /// intercept rabbitmq masstransit configuration
    /// and inject tracing
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "MassTransit.RabbitMqTransport", TargetMethodName = "CreateUsingRabbitMq", TargetTypeName = "MassTransit.BusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
    public class CreateUsingRabbitMqInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            //Console.WriteLine("Masstransit configuration Intercepted");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var config = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            //Console.WriteLine(config);

            var typedConfigure = (Action<IRabbitMqBusFactoryConfigurator>)GetParameter(1);
            Action<IRabbitMqBusFactoryConfigurator> configure = (IRabbitMqBusFactoryConfigurator cfg) => {
                //Console.WriteLine("Masstransit configuration Injected");

                if (config.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => {
                        configurator.ConnectPublishPipeSpecificationObserver(new OpenTracingPublishPipeSpecificationObserver());
                    });
                }

                typedConfigure(cfg);
            };

            ModifyParameter(1, configure);
        }
    }
}
