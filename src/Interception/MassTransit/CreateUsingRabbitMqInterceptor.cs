using Interception.Attributes;
using Interception.Base;
using MassTransit.RabbitMqTransport;
using Microsoft.Extensions.Configuration;
using System;

namespace Interception.MassTransit
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit.RabbitMqTransport", TargetMethodName = "CreateUsingRabbitMq", TargetTypeName = "MassTransit.BusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
    public class CreateUsingRabbitMqInterceptor : BaseInterceptor
    {
        protected override void ExecuteAfter(object result, Exception exception)
        {
        }

        protected override void ExecuteBefore()
        {
            Console.WriteLine("Masstransit configuration Intercepted");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var config = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            Console.WriteLine(config);

            var typedConfigure = (Action<IRabbitMqBusFactoryConfigurator>)_parameters[1];
            Action<IRabbitMqBusFactoryConfigurator> myConfigure = (IRabbitMqBusFactoryConfigurator cfg) => {
                Console.WriteLine("Masstransit configuration Injected");

                if (config.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => {
                        configurator.ConnectPublishPipeSpecificationObserver(new OpenTracingPublishPipeSpecificationObserver());
                    });
                }

                typedConfigure(cfg);
            };

            _parameters = new System.Collections.Generic.List<object> { _parameters[0], myConfigure };
        }
    }
}
