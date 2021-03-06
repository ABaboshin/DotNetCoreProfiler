﻿using Interception.Attributes;
using Interception.Attributes.Cache;
using Interception.Core;
using Microsoft.Extensions.DependencyInjection;

namespace Interception.Cache
{
    [MethodInterceptorImplementation(typeof(InvalidateCacheAttribute))]
    public class InvalidateCacheInterceptor : BaseAttributedInterceptor
    {
        public override int Priority => 100;

        public override void ExecuteBefore()
        {
            var attribute = GetCustomAttribute<InvalidateCacheAttribute>();

            var cacheInvalidator = DependencyInjection.Instance.ServiceProvider.GetRequiredService<IDistributedCacheInvalidator>();
            cacheInvalidator.Invalidate(attribute.Name);
        }
    }
}
