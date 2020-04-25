using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Options;
using StackExchange.Redis;
using System;
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
            _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .Subscribe("channel1", (channel, data) => {
                    Thread.Sleep(3000);
                    Console.WriteLine($"RedisSubscriber sync1 channel {channel} data: {data}");
                });

            _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .Subscribe("channel2")
                .OnMessage(async data =>
                {
                    await Task.Delay(3000);
                    Console.WriteLine($"RedisSubscriber async4 data: {data}");
                });
        }
    }
}
