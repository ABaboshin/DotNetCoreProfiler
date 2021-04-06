using OpenTracing.Propagation;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Http;

namespace Interception.Observers
{
    public class RequestHeadersInjectAdapter : ITextMap
    {
        private readonly HttpRequestMessage _request;

        public RequestHeadersInjectAdapter(HttpRequestMessage request)
        {
            _request = request;
        }

        public IEnumerator<KeyValuePair<string, string>> GetEnumerator()
        {
            throw new NotImplementedException();
        }

        public void Set(string key, string value)
        {
            if (_request.Headers.Contains(key))
            {
                _request.Headers.Remove(key);
            }

            _request.Headers.Add(key, value);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
