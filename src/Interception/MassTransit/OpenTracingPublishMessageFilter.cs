using GreenPipes;
using Interception.AspNetCore;
using MassTransit;
using Microsoft.Extensions.Options;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Threading.Tasks;
using Microsoft.Extensions.DependencyInjection;

namespace Interception.MassTransit
{
    public class OpenTracingPublishMessageFilter<TMessage> : IFilter<PublishContext<TMessage>> where TMessage : class
    {
        public void Probe(ProbeContext context)
        {
        }

        public async Task Send(PublishContext<TMessage> context, IPipe<PublishContext<TMessage>> next)
        {
            Console.WriteLine($"PublishFilter {GlobalTracer.Instance.ActiveSpan != null}");

            var baseSpan = GlobalTracer.Instance
                .BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<MassTransitConfiguration>>().Value.PublisherName)
                .AsChildOf(GlobalTracer.Instance.ActiveSpan);

            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindProducer)
                    .SetTag("MessageId", context.MessageId?.ToString())
                    .SetTag("MessageType", context.Message.GetType().FullName);

                GlobalTracer.Instance.Inject(span.Context, BuiltinFormats.TextMap, new MassTransitTextMapInjectAdapter(context));

                await next.Send(context);
            }
        }
    }
}