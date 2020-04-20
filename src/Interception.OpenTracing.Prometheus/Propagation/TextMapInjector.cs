using OpenTracing;
using OpenTracing.Propagation;
using System;

namespace Interception.OpenTracing.Prometheus.Propagation
{
    public class TextMapInjector : IInjector<ITextMap>
    {
        private readonly string _spanContextKey;

        public TextMapInjector(string spanContextKey)
        {
            _spanContextKey = spanContextKey;
        }

        public void Inject(ISpanContext spanContext, ITextMap carrier)
        {
            Console.WriteLine($"TextMapInjector {_spanContextKey}={spanContext}");

            carrier.Set(_spanContextKey, spanContext.ToString());

            foreach (var baggage in spanContext.GetBaggageItems())
            {
                Console.WriteLine($"TextMapInjector {baggage.Key}={baggage.Value}");

                carrier.Set(baggage.Key, baggage.Value);
            }
        }
    }
}
