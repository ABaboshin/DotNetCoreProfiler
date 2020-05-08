using Interception.Attributes;
using MassTransit;
using Microsoft.Extensions.Logging;
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace SampleApp.MessageBus
{
    public class MyMessageConsumer :
        IConsumer<MyMessage>
    {
        private readonly ILogger<MyMessageConsumer> _logger;

        public MyMessageConsumer(ILogger<MyMessageConsumer> logger)
        {
            _logger = logger;
        }

        public async Task Consume(ConsumeContext<MyMessage> context)
        {
            _logger.LogInformation("MyMessageConsumer.Consume");
            Console.WriteLine("Start consuming");

            var test = await LongAction(new { x = 1 }, "test");

            Console.WriteLine($"test {test}");

            var client = new HttpClient();

            var request = new HttpRequestMessage
            {
                Method = HttpMethod.Get,
                RequestUri = new Uri("http://localhost:5000/api/values")
            };

            var response = await client.SendAsync(request);

            FibonacciCall();

            FibonacciCall();

            Console.WriteLine("Done consuming");
        }

        [Monitor(Name = "Fibonacci call")]
        void FibonacciCall()
        {
            Fibonacci(100);
        }


        [Monitor(Name = "A long time action", Parameters = new[] { "o1" }, ReturnValue = true)]
        async Task<int> LongAction(object o1, object o2)
        {
            await Task.Delay(3000);
            return 27;
        }

        [Cache(DurationSeconds = 60, Parameters = new[] { "n" })]
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
    }
}
