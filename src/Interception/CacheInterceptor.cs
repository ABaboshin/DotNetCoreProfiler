using Interception.Attributes;
using Interception.Base;
using Microsoft.Extensions.Caching.Memory;
using Microsoft.Extensions.Options;
using System;
using System.Linq;

namespace Interception
{
    [CacheInterceptor]
    public class CacheInterceptor : BaseInterceptor
    {
        private static MemoryCache _cache = new MemoryCache(Options.Create(new MemoryCacheOptions()));

        string GetCacheKey()
        {
            var method = _methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
            var attribute = (CacheAttribute)method.GetCustomAttributes(typeof(CacheAttribute), false).FirstOrDefault();

            var key = method.Name;

            if (attribute.Parameters != null && attribute.Parameters.Any())
            {
                var methodParameters = method.GetParameters().ToList();

                foreach (var p in attribute.Parameters)
                {
                    var index = methodParameters.FindIndex(mp => mp.Name == p);
                    if (index != -1)
                    {
                        key += "-" + _parameters[index]?.ToString();
                    }
                }
            }

            return key;
        }

        protected override void ExecuteAfter(object result, Exception exception)
        {
            _cache.Set(GetCacheKey(), result);
        }

        protected override void ExecuteBefore()
        {
        }

        public override object Execute()
        {
            _cache.TryGetValue(GetCacheKey(), out object result);
            if (result is null)
            {
                return base.Execute();
            }

            return result;
        }
    }
}
