using OpenTracing;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Prometheus
{
    class SpanContext : ISpanContext
    {
        private readonly IDictionary<string, string> _baggage = new Dictionary<string, string>();

        public string TraceId { get; }

        public string SpanId { get; }

        public string ParentSpanId { get; }

        public SpanContext(string traceId, string spanId, string parentSpanId, IDictionary<string, string> baggage)
        {
            TraceId = traceId;
            SpanId = spanId;
            ParentSpanId = parentSpanId;
            _baggage = baggage;
        }

        public SpanContext(string value)
        {
            var parts = value.Split(':');
            if (parts.Length < 2)
            {
                throw new ArgumentException($"Wrong SpanContext representation {value}");
            }

            TraceId = parts[0];
            SpanId = parts[1];
        }

        public IEnumerable<KeyValuePair<string, string>> GetBaggageItems()
        {
            return _baggage;
        }

        public SpanContext WithBaggage(Dictionary<string, string> newBaggage)
        {
            return new SpanContext(TraceId, SpanId, ParentSpanId, newBaggage);
        }

        public override string ToString()
        {
            return $"{TraceId}:{SpanId}";
        }

        internal SpanContext WithBaggageItem(string key, string value)
        {
            var baggage = new Dictionary<string, string>(_baggage);
            if (baggage.Any(b => b.Key == key))
            {
                baggage.Remove(key);
            }

            baggage.Add(key, value);

            return WithBaggage(baggage);
        }
    }
}
