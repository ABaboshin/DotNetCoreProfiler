using Interception.Tracing.Extensions;
using MassTransit;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Linq;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1[!1]", TargetMethodParametersCount = 1)]
    public class MassTransitConsumerInterceptor : BaseInterceptor
    {
        public object Execute()
        {
            var method = _this.GetType().GetMethod("Consume");

            return Execute(async () => {
                var task = (Task)method.Invoke(_this, _parameters.ToArray());
                await task;
            }, _this.GetType().FullName, (ConsumeContext)_parameters[0]);
        }

        private async Task Execute(Func<Task> action, string consumerName, ConsumeContext context)
        {
            if (!MassTransitInterception.MassTransitConfiguration.ConsumerEnabled)
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
                    .BuildSpan(MassTransitInterception.MassTransitConfiguration.ConsumerName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                    .AsChildOf(parentSpanContext);
            }
            catch (Exception)
            {
                spanBuilder = GlobalTracer.Instance.BuildSpan(MassTransitInterception.MassTransitConfiguration.ConsumerName);
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

            Console.WriteLine("Done consuming interceptor");
        }
    }
}
