using Interception.AspNetCore;
using Interception.Attributes;
using Interception.Base;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Linq;

namespace Interception.Cache
{
    [CacheInterceptor]
    public class CacheInterceptor : BaseInterceptor
    {
        IDistributedCache DistributedCache => DependencyInjection.ServiceProvider.GetRequiredService<IDistributedCache>();

        string GetCacheKey()
        {
            var method = FindMethod();
            var attribute = (CacheAttribute)method.GetCustomAttributes(typeof(CacheAttribute), false).First();

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
            Console.WriteLine($"Cache.ExecuteAfter {DateTime.UtcNow}");

            var method = FindMethod();
            var attribute = (CacheAttribute)method.GetCustomAttributes(typeof(CacheAttribute), false).First();

            DistributedCache.Set(GetCacheKey(), Serialization.ToByteArray(result), new DistributedCacheEntryOptions { AbsoluteExpirationRelativeToNow = TimeSpan.FromSeconds(attribute.DurationSeconds) });
        }

        protected override void ExecuteBefore()
        {
        }

        public override object Execute()
        {
            Console.WriteLine($"Cache.Execute {DateTime.UtcNow} {DistributedCache}");
            var cached = DistributedCache.Get(GetCacheKey());

            Console.WriteLine($"Cache.Execute {DateTime.UtcNow} cache {cached != null}");

            if (cached != null)
            {
                return base.Execute();
            }

            Console.WriteLine($"Cache.Execute {DateTime.UtcNow} cached result");

            DistributedCache.Refresh(GetCacheKey());

            return Serialization.FromByteArray(cached);
        }
    }
}
