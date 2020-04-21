using OpenTracing;
using OpenTracing.Propagation;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Prometheus.Propagation
{
    class TextMapExtractor : IExtractor<ITextMap>
    {
        private readonly string _spanContextKey;

        public TextMapExtractor(string spanContextKey)
        {
            _spanContextKey = spanContextKey;
        }

        public ISpanContext Extract(ITextMap carrier)
        {
            SpanContext spanContext = null;
            Dictionary<string, string> baggage = new Dictionary<string, string>();
            foreach (var item in carrier)
            {
                if (item.Key.ToLower() == _spanContextKey.ToLower())
                {
                    spanContext = new SpanContext(item.Value);
                }

                baggage.Add(item.Key, item.Value);
            }

            if (spanContext is null)
            {
                return null;
            }

            if (baggage.Any())
            {
                spanContext = spanContext.WithBaggage(baggage);
            }

            return spanContext;
        }
    }
}
