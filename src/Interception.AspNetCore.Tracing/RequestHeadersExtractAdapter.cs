using Microsoft.AspNetCore.Http;
using OpenTracing.Propagation;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Interception.AspNetCore
{
    internal class RequestHeadersExtractAdapter : ITextMap
    {
        private readonly HttpContext _context;

        public RequestHeadersExtractAdapter(HttpContext context)
        {
            _context = context;
        }

        public void Set(string key, string value)
        {
            throw new NotSupportedException("This class should only be used with ITracer.Extract");
        }

        public IEnumerator<KeyValuePair<string, string>> GetEnumerator()
        {
            foreach (var item in _context.Request.Headers.Where(k => k.Key.ToLower().Contains("-id")))
            {
                yield return new KeyValuePair<string, string>(item.Key, item.Value);
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}