using Microsoft.AspNetCore.Http;
using OpenTracing.Propagation;
using System;
using System.Collections;
using System.Collections.Generic;

namespace Interception.Observers.Http
{
    internal class RequestHeadersExtractAdapter : ITextMap
    {
        private readonly HttpContext _context;

        public RequestHeadersExtractAdapter(HttpContext context)
        {
            _context = context ?? throw new ArgumentNullException(nameof(context));
        }

        public void Set(string key, string value)
        {
            throw new NotSupportedException("This class should only be used with ITracer.Extract");
        }

        public IEnumerator<KeyValuePair<string, string>> GetEnumerator()
        {
            //foreach (var kvp in _context.Request.Headers)
            //{
            //    yield return new KeyValuePair<string, string>(kvp.Key, kvp.Value);
            //}

            yield return new KeyValuePair<string, string>("traceIdentifier", _context.TraceIdentifier);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}