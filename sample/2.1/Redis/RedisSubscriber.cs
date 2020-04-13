using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Options;
using StackExchange.Redis;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace SampleApp.Redis
{
    public class RedisSubscriber : BackgroundService
    {
        private readonly ConnectionMultiplexer _connectionMultiplexer;

        public RedisSubscriber(IOptions<RedisConfiguration> options)
        {
            _connectionMultiplexer = ConnectionMultiplexer.Connect(options.Value.ConnectionString);
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            await _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .SubscribeAsync("channel", (channel, data) => {
                    Console.WriteLine($"channel: {channel} data: {data}");
                });
        }
    }
}
