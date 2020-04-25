using Interception.Common;
using Interception.Tracing.Extensions;
using MassTransit;
using MassTransit.RabbitMqTransport;
using Microsoft.Extensions.Configuration;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    public static class MassTransitInterception
    {
        public static MassTransitConfiguration MassTransitConfiguration;

        [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit.RabbitMqTransport", TargetMethodName = "CreateUsingRabbitMq", TargetTypeName = "MassTransit.BusFactoryConfiguratorExtensions", TargetMethodParametersCount = 2)]
        public static object CreateUsingRabbitMq(object selector, object configure, int mdToken, long moduleVersionPtr)
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            MassTransitConfiguration = configuration.GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>();

            var typedConfigure = (Action<IRabbitMqBusFactoryConfigurator>)configure;
            Action<IRabbitMqBusFactoryConfigurator> myConfigure = (IRabbitMqBusFactoryConfigurator cfg) => {
                Console.WriteLine("Masstransit configuration Injected");

                if (MassTransitConfiguration.PublisherEnabled)
                {
                    cfg.ConfigurePublish(configurator => configurator.AddPipeSpecification(new OpenTracingPipeSpecification()));
                }

                typedConfigure(cfg);
            };

            return MethodExecutor.ExecuteMethod(null, new object[] { selector, myConfigure }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1[!1]", TargetMethodParametersCount = 1)]
        public static object Consume(object consumer, object context, int mdToken, long moduleVersionPtr)
        {
            var method = consumer.GetType().GetMethod("Consume");

            return Execute(async () => {
                var task = (Task)method.Invoke(consumer, new[] { context });
                await task;
            }, consumer.GetType().FullName, (ConsumeContext)context);
        }

        private static async Task Execute(Func<Task> action, string consumerName, ConsumeContext context)
        {
            if (!MassTransitConfiguration.ConsumerEnabled)
            {
                await action();
                return;
            }

            ISpanBuilder spanBuilder;

            try
            {
                var headers = context.Headers.GetAll().ToDictionary(pair => pair.Key, pair => pair.Value.ToString());
                var parentSpanContext = GlobalTracer.Instance.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(headers));

                spanBuilder = GlobalTracer.Instance
                    .BuildSpan(MassTransitConfiguration.ConsumerName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                    .AsChildOf(parentSpanContext);
            }
            catch (Exception)
            {
                spanBuilder = GlobalTracer.Instance.BuildSpan(MassTransitConfiguration.ConsumerName);
            }

            spanBuilder
                .WithTag("consumer", consumerName)
                .WithTag("message-id", context.MessageId?.ToString());

            using (spanBuilder.StartActive(true))
            {
                try
                {
                    await action();
                }
                catch (Exception ex)
                {
                    GlobalTracer.Instance.ActiveSpan.SetException(ex);
                    throw;
                }
            }
        }
    }
}
