using GreenPipes;
using MassTransit;
using OpenTracing.Propagation;
using OpenTracing.Util;
using System;
using System.Threading.Tasks;

namespace Interception.MassTransit
{
    public class OpenTracingPublishFilter : IFilter<PublishContext>
    {
        public void Probe(ProbeContext context)
        { }

        public async Task Send(PublishContext context, IPipe<PublishContext> next)
        {
            //Console.WriteLine("OpenTracingPublishFilter 1");
            //var operationName = $"Publishing Message: {context.DestinationAddress}";
            //Console.WriteLine("OpenTracingPublishFilter 2");

            //var spanBuilder = GlobalTracer.Instance.BuildSpan(operationName)
            //   .AsChildOf(GlobalTracer.Instance.ActiveSpan.Context)
            //   .WithTag("destination-address", context.DestinationAddress?.ToString())
            //   .WithTag("source-address", context.SourceAddress?.ToString())
            //   .WithTag("initiator-id", context.InitiatorId?.ToString())
            //   .WithTag("message-id", context.MessageId?.ToString());

            //Console.WriteLine("OpenTracingPublishFilter 3");

            //using (var scope = spanBuilder.StartActive())
            //{
            //    Console.WriteLine("OpenTracingPublishFilter 4");

            //    GlobalTracer.Instance.Inject(
            //       GlobalTracer.Instance.ActiveSpan.Context,
            //       BuiltinFormats.TextMap,
            //       new MassTransitTextMapInjectAdapter(context));

            //    Console.WriteLine("OpenTracingPublishFilter 5");

                await next.Send(context);

            //    Console.WriteLine("OpenTracingPublishFilter 6");
            //}
        }
    }
}