using OpenTracing;
using OpenTracing.Propagation;
using System;
using System.Collections;
using System.Collections.Generic;

namespace Interception.StackExchangeRedis
{
    public class StackExchangeRedisTextMapInjectAdapter : ITextMap
    {
        private readonly Dictionary<string, string> _context;
        public StackExchangeRedisTextMapInjectAdapter(Dictionary<string, string> context)
        {
            _context = context;
        }

        public IEnumerator<KeyValuePair<string, string>> GetEnumerator()
        {
            throw new NotSupportedException(
                $"{nameof(TextMapInjectAdapter)} should only be used with {nameof(ITracer)}.{nameof(ITracer.Inject)}");
        }

        public void Set(string key, string value)
        {
            _context.Add(key, value);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
