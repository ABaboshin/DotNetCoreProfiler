using Interception.Attributes;
using Interception.Core;
using MassTransit;
using Microsoft.Extensions.Configuration;
using System;

namespace Interception.MassTransit
{
    /// <summary>
    /// intercept rabbitmq masstransit configuration
    /// and inject tracing
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "UsingRabbitMq", TargetTypeName = "MassTransit.RabbitMqBusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
    public class CreateUsingRabbitMqInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var config = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            var typedConfigure = (Action<IBusRegistrationContext, IRabbitMqBusFactoryConfigurator>)GetParameter(1);
            Action<IBusRegistrationContext, IRabbitMqBusFactoryConfigurator> configure = (IBusRegistrationContext context, IRabbitMqBusFactoryConfigurator cfg) => {
                if (config.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => {
                        configurator.ConnectPublishPipeSpecificationObserver(new OpenTracingPublishPipeSpecificationObserver());
                    });
                }

                typedConfigure(context, cfg);
            };

            ModifyParameter(1, configure);
        }
    }
}
