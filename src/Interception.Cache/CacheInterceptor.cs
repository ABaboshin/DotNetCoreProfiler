using Interception.Attributes;
using Interception.Attributes.Cache;
using Interception.Core;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Linq;

namespace Interception.Cache
{
    /// <summary>
    /// cache interceptor
    /// </summary>
    [MethodInterceptorImplementation(typeof(CacheAttribute))]
    public class CacheInterceptor : BaseAttributedInterceptor
    {
        IDistributedCache DistributedCache => DependencyInjection.Instance.ServiceProvider.GetRequiredService<IDistributedCache>();

        public override int Priority => 100;

        string GetCacheKey()
        {
            var attribute = GetCustomAttribute<CacheAttribute>();

            var key = Method.Name;

            if (attribute.Parameters != null && attribute.Parameters.Any())
            {
                var methodParameters = Method.GetParameters().ToList();

                foreach (var p in attribute.Parameters)
                {
                    var index = methodParameters.FindIndex(mp => mp.Name == p);
                    if (index != -1)
                    {
                        key += "-" + GetParameter(index)?.ToString();
                    }
                }
            }

            return key;
        }

        public override void ExecuteAfter()
        {
            Console.WriteLine($"Cache.ExecuteAfter {DateTime.UtcNow} {GetCacheKey()}");

            var attribute = (CacheAttribute)Method.GetCustomAttributes(typeof(CacheAttribute), false).First();

            DistributedCache.Set(GetCacheKey(), Serialization.ToByteArray(Result), new DistributedCacheEntryOptions { AbsoluteExpirationRelativeToNow = TimeSpan.FromSeconds(attribute.DurationSeconds) });
        }

        public override bool SkipExecution()
        {
            var cached = DistributedCache.Get(GetCacheKey());
            if (cached != null)
            {
                DistributedCache.Refresh(GetCacheKey());
                Result = Serialization.FromByteArray(cached);
                Console.WriteLine($"Cache.SkipExecution {GetCacheKey()}");
                return true;
            }
            
            Console.WriteLine($"Cache.SkipExecution {GetCacheKey()} no");

            return false;
        }
    }
}
