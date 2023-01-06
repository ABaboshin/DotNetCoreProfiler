using Interception.Attributes;
using MassTransit;
using Microsoft.Extensions.Configuration;
using System;
using System.Runtime.CompilerServices;

namespace Interception.MassTransit
{
    /// <summary>
    /// intercept rabbitmq masstransit configuration
    /// and inject tracing
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "UsingRabbitMq", TargetTypeName = "MassTransit.RabbitMqBusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
    public class CreateUsingRabbitMqInterceptor
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Before<TType, T1, T2>(TType instance, ref T1 a1, ref T2 a2)
        {
            
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var config = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            var typedConfigure = (Action<IBusRegistrationContext, IRabbitMqBusFactoryConfigurator>)(object)a2;
            Action<IBusRegistrationContext, IRabbitMqBusFactoryConfigurator> configure = (IBusRegistrationContext context, IRabbitMqBusFactoryConfigurator cfg) => {
                if (config.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => {
                        configurator.ConnectPublishPipeSpecificationObserver(new OpenTracingPublishPipeSpecificationObserver());
                    });
                }

                typedConfigure(context, cfg);
            };

            a2 = (T2)(object)configure;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void After<TResult>(TResult result, Exception ex)
        {
        }
    }
}
