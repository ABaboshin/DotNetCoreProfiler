using Interception.Attributes;
using MassTransit;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
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

            var longAction = GetType().GetMethods(System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance).Where(m => m.Name == "LongAction").FirstOrDefault();
            var task = (Task<int>)Executor.Execute(this, new List<object> { new { x = 1 }, "test" }, longAction);
            await task;


            var test = await LongAction(new { x = 1 }, "test");

            Console.WriteLine($"test {test}");

            var client = new HttpClient();

            var request = new HttpRequestMessage
            {
                Method = HttpMethod.Get,
                RequestUri = new Uri("http://localhost:5000/api/values")
            };

            var response = await client.SendAsync(request);

            Console.WriteLine("Done consuming");
        }

        [Monitor(Name = "Test", Parameters = new[] { "o2" }, ReturnValue = true)]
        async Task<int> LongAction(object o1, object o2)
        {
            await Task.Delay(3000);
            return 3;
        }
    }
}
