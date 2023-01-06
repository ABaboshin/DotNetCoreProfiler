using MassTransit;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System.Threading.Tasks;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using GreenPipes;

namespace Interception.MassTransit
{
    public class OpenTracingPublishMessageFilter<TMessage> : IFilter<PublishContext<TMessage>> where TMessage : class
    {
        public void Probe(ProbeContext context)
        {
        }

        public async Task Send(PublishContext<TMessage> context, IPipe<PublishContext<TMessage>> next)
        {
            var baseSpan = GlobalTracer.Instance
                .BuildSpan(DependencyInjection.Instance.ServiceProvider.GetService<IConfiguration>().GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>().PublisherName)
                .AsChildOf(GlobalTracer.Instance.ActiveSpan);

            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindProducer)
                    .SetTag("MessageId", context.MessageId?.ToString())
                    .SetTag("MessageType", context.Message.GetType().FullName);

                GlobalTracer.Instance.Inject(span.Context, BuiltinFormats.TextMap, new TextMapInjectAdapter(context));

                await next.Send(context);
            }
        }
    }
}