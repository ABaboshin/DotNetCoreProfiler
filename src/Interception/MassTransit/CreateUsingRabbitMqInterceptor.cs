using Interception.Attributes;
using MassTransit.RabbitMqTransport;
using Microsoft.Extensions.Configuration;
using System;

namespace Interception.MassTransit
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit.RabbitMqTransport", TargetMethodName = "CreateUsingRabbitMq", TargetTypeName = "MassTransit.BusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
    public class CreateUsingRabbitMqInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            Console.WriteLine("Masstransit configuration Intercepted");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            MassTransitInterception.MassTransitConfiguration = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            Console.WriteLine(MassTransitInterception.MassTransitConfiguration);

            var typedConfigure = (Action<IRabbitMqBusFactoryConfigurator>)_parameters[1];
            Action<IRabbitMqBusFactoryConfigurator> myConfigure = (IRabbitMqBusFactoryConfigurator cfg) => {
                Console.WriteLine("Masstransit configuration Injected");

                if (MassTransitInterception.MassTransitConfiguration.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => configurator.AddPipeSpecification(new OpenTracingPipeSpecification()));
                }

                typedConfigure(cfg);
            };

            _parameters = new System.Collections.Generic.List<object> { _parameters[0], myConfigure };

            return ExecuteInternal(false);
        }
    }
}
