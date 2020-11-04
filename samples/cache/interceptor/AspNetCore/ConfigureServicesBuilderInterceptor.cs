using Interception.Attributes;
using Interception.Cache;
using Interception.Cache.Redis;
using Interception.Core;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Caching.Distributed;
using Microsoft.Extensions.Caching.StackExchangeRedis;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Options;
using System;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept Startup.ConfigureService and inject cache
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "InvokeCore", TargetTypeName = "Microsoft.AspNetCore.Hosting.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureServicesBuilderInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var serviceCollection = ((IServiceCollection)GetParameter(1));
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            ConfigureCache(serviceCollection, configuration);
        }

        private void ConfigureCache(IServiceCollection serviceCollection, IConfigurationRoot configuration)
        {
            serviceCollection.Configure<CacheConfiguration>(configuration.GetSection(CacheConfiguration.SectionKey));
            serviceCollection.AddSingleton<IDistributedCache>(sp => {
                var ccOptions = sp.GetRequiredService<IOptions<CacheConfiguration>>();
                if (ccOptions.Value.Type == "redis")
                {
                    return new RedisCache(Options.Create(new RedisCacheOptions { Configuration = ccOptions.Value.Configuration }));
                }

                return null;
            });
            serviceCollection.AddSingleton<IDistributedCacheInvalidator>(sp => {
                var ccOptions = sp.GetRequiredService<IOptions<CacheConfiguration>>();
                if (ccOptions.Value.Type == "redis")
                {
                    return new RedisCacheInvalidator(Options.Create(new RedisCacheOptions { Configuration = ccOptions.Value.Configuration }));
                }

                return null;
            });
        }
    }
}
