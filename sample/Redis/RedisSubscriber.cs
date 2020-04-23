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

            await _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .SubscribeAsync("channel2", async (channel, data) => {
                    await Task.Delay(3000);
                    Console.WriteLine($"RedisSubscriber async2 channel {channel} data: {data}");
                });

            _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .Subscribe("channel3")
                .OnMessage(data =>
                {
                    Thread.Sleep(3000);
                    Console.WriteLine($"RedisSubscriber sync3 data: {data}");
                });

            _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .Subscribe("channel4")
                .OnMessage(async data =>
                {
                    await Task.Delay(3000);
                    Console.WriteLine($"RedisSubscriber async4 data: {data}");
                });
        }
    }
}
