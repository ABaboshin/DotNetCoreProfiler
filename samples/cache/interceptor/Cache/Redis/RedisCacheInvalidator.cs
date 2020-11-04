using Microsoft.Extensions.Caching.StackExchangeRedis;
using Microsoft.Extensions.Options;
using StackExchange.Redis;
using System;
using System.Threading;

namespace Interception.Cache.Redis
{
    public class RedisCacheInvalidator : IDistributedCacheInvalidator
    {
        private const string ClearScript = (@"
                for _,k in ipairs(redis.call('KEYS', ARGV[1])) do
                     redis.call('DEL', k)
                end
                return 1");

        private readonly RedisCacheOptions _options;
        private readonly string _instance;

        private volatile ConnectionMultiplexer _connection;
        private IDatabase _cache;
        private readonly SemaphoreSlim _connectionLock = new SemaphoreSlim(initialCount: 1, maxCount: 1);

        public RedisCacheInvalidator(IOptions<RedisCacheOptions> optionsAccessor)
        {
            if (optionsAccessor == null)
            {
                throw new ArgumentNullException(nameof(optionsAccessor));
            }

            _options = optionsAccessor.Value;

            // This allows partitioning a single backend cache for use with multiple apps/services.
            _instance = _options.InstanceName ?? string.Empty;
        }

        public void Invalidate(string prefix)
        {
            Connect();

            _cache.ScriptEvaluate(ClearScript,
                values: new RedisValue[]
                {
                        _instance + prefix + "*"
                });
        }

        private void Connect()
        {
            if (_cache != null)
            {
                return;
            }

            _connectionLock.Wait();
            try
            {
                if (_cache == null)
                {
                    if (_options.ConfigurationOptions != null)
                    {
                        _connection = ConnectionMultiplexer.Connect(_options.ConfigurationOptions);
                    }
                    else
                    {
                        _connection = ConnectionMultiplexer.Connect(_options.Configuration);
                    }
                    _cache = _connection.GetDatabase();
                }
            }
            finally
            {
                _connectionLock.Release();
            }
        }
    }
}
