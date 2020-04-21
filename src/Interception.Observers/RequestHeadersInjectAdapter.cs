using OpenTracing.Propagation;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Http.Headers;

namespace Interception.Observers
{
    public class RequestHeadersInjectAdapter : ITextMap
    {
        private readonly HttpHeaders _headers;

        public RequestHeadersInjectAdapter(HttpHeaders headers)
        {
            _headers = headers;
        }

        public IEnumerator<KeyValuePair<string, string>> GetEnumerator()
        {
            throw new NotImplementedException();
        }

        public void Set(string key, string value)
        {
            Console.WriteLine($"SET {key} {value}");

            if (_headers.Contains(key))
            {
                _headers.Remove(key);
            }

            _headers.Add(key, value);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
