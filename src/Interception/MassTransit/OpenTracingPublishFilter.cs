using GreenPipes;
using MassTransit;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    public class OpenTracingPublishFilter : IFilter<PublishContext>
    {
        public void Probe(ProbeContext context)
        {
        }

        public async Task Send(PublishContext context, IPipe<PublishContext> next)
        {
            if (!MassTransitInterception.MassTransitConfiguration.PublisherEnabled)
            {
                await next.Send(context);
                return;
            }

            var baseSpan = Tracing.Tracing.Tracer
                .BuildSpan(MassTransitInterception.MassTransitConfiguration.PublisherName)
                .AsChildOf(Tracing.Tracing.CurrentScope?.Span);
            
            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindProducer)
                    .SetTag("message-id", context.MessageId?.ToString());

                Interception.Tracing.Tracing.Tracer.Inject(span.Context, BuiltinFormats.TextMap, new MassTransitTextMapInjectAdapter(context));

                await next.Send(context);
            }
        }
    }
}