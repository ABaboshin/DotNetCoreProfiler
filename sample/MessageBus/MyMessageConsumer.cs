using Interception.Attributes;
using Interception.Attributes.Cache;
using Interception.Attributes.Validation;
using MassTransit;
using Microsoft.Extensions.Logging;
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace SampleApp.MessageBus
{
    public class MyMessageConsumer :
        IConsumer<MyMessage>,
        IConsumer<MyBadMessage>
    {
        private readonly ILogger<MyMessageConsumer> _logger;

        public MyMessageConsumer(ILogger<MyMessageConsumer> logger)
        {
            _logger = logger;
        }

        public async Task Consume(ConsumeContext<MyMessage> context)
        {
            _logger.LogInformation("MyMessageConsumer.Consume");

            var test = await LongAction(new { x = 1 }, "test");

            _logger.LogInformation("test {@test}", test);

            var client = new HttpClient();

            Console.WriteLine($"{Environment.GetEnvironmentVariable("SERVICE_URL")}/api/values/publish/1");
            var request = new HttpRequestMessage
            {
                Method = HttpMethod.Get,
                RequestUri = new Uri($"{Environment.GetEnvironmentVariable("SERVICE_URL")}/api/values/publish")
            };

            var response = await client.SendAsync(request);

            Fibonacci30(1);

            Fibonacci30(2);

            DummyCache();

            InvalidateCache();

            _logger.LogInformation("MyMessageConsumer.Consume.Done");
        }


        [Monitor(Name = "InvalidateCache")]
        [InvalidateCache(Name = nameof(DummyCache))]
        void InvalidateCache()
        {
        }

        [Cache(DurationSeconds = 6000)]
        int DummyCache()
        {
            return 3;
        }

        [Cache(DurationSeconds = 6000, Parameters = new[] { "n" })]
        [Monitor(Name = "Fibonacci call")]
        int Fibonacci30([GreatThenZero]int n)
        {
            return Fibonacci(30);
        }

        [Monitor(Name = "A long time action", Parameters = new[] { "o1" }, ReturnValue = true)]
        async Task<int> LongAction(object o1, object o2)
        {
            await Task.Delay(3000);
            return 27;
        }
        
        int Fibonacci(int n)
        {
            if (n < 0)
            {
                throw new ArgumentOutOfRangeException();
            }

            if (n == 0)
            {
                return 0;
            }

            if (n == 1)
            {
                return 1;
            }

            return Fibonacci(n - 1) + Fibonacci(n - 2);
        }

        public async Task Consume(ConsumeContext<MyBadMessage> context)
        {
            await Task.Delay(1000);
            throw new NotImplementedException("Oops");
        }
    }
}
