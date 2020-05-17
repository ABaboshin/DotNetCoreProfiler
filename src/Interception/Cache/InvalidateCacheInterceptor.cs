using Interception.AspNetCore;
using Interception.Attributes;
using Interception.Core;
using Microsoft.Extensions.DependencyInjection;

namespace Interception.Cache
{
    [MethodInterceptorImplementation(typeof(InvalidateCacheAttribute))]
    public class InvalidateCacheInterceptor : BaseAttributedInterceptor
    {
        public override void ExecuteBefore()
        {
            var attribute = GetCustomAttribute<InvalidateCacheAttribute>();

            var cacheInvalidator = DependencyInjection.ServiceProvider.GetRequiredService<IDistributedCacheInvalidator>();
            cacheInvalidator.Invalidate(attribute.Name);
        }
    }
}
