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
            await LongAction();

            var client = new HttpClient();

            var request = new HttpRequestMessage
            {
                Method = HttpMethod.Get,
                RequestUri = new Uri("http://localhost:5000/api/values")
            };

            var response = await client.SendAsync(request);

            Console.WriteLine("Done consuming");
        }

        [Monitor()]
        async Task LongAction()
        {
            await Task.Delay(3000);
        }
    }
}
