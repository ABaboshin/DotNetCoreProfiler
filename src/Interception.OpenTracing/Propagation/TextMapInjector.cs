using OpenTracing;
using OpenTracing.Propagation;
using System;

namespace Interception.OpenTracing.Propagation
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
            carrier.Set(_spanContextKey, spanContext.ToString());

            foreach (var baggage in spanContext.GetBaggageItems())
            {
                carrier.Set(baggage.Key, baggage.Value);
            }
        }
    }
}
