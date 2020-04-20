using GreenPipes;
using MassTransit;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    public class OpenTracingPublishFilter : IFilter<PublishContext>
    {
        public void Probe(ProbeContext context)
        { }

        public async Task Send(PublishContext context, IPipe<PublishContext> next)
        {
            var baseSpan = Tracing.Tracing.Tracer
                .BuildSpan("publish-rabbitmq")
                .AsChildOf(Tracing.Tracing.CurrentScope?.Span);
            
            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindClient)
                    .SetTag("message-id", context.MessageId?.ToString());

                var dictionary = new Dictionary<string, string>();
                Interception.Tracing.Tracing.Tracer.Inject(span.Context, BuiltinFormats.TextMap, new MassTransitTextMapInjectAdapter(context));

                foreach (var item in dictionary)
                {
                    context.Headers.Set(item.Key, item.Value);
                }

                await next.Send(context);
            }
        }
    }
}