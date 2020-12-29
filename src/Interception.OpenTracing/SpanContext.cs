using OpenTracing;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing
{
    public class SpanContext : ISpanContext
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

        public static SpanContext FromString(string value)
        {
            var parts = value.Split(':');
            if (parts.Length != 3)
            {
                return null;
            }

            return new SpanContext(parts[0], parts[1], parts[2], new Dictionary<string, string>());
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
            return $"{TraceId}:{SpanId}:{ParentSpanId}";
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
