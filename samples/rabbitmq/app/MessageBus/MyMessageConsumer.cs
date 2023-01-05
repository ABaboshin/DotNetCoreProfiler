using MassTransit;
using Microsoft.Extensions.Logging;
using System;
using System.Threading.Tasks;

namespace app.MessageBus
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

            _logger.LogInformation("MyMessageConsumer.Consume.Done");
        }


        // [Monitor(Name = "A long time action", Parameters = new[] { "o1" }, ReturnValue = true)]
        async Task<int> LongAction(object o1, object o2)
        {
            await Task.Delay(3000);
            return 27;
        }

        public async Task Consume(ConsumeContext<MyBadMessage> context)
        {
            await Task.Delay(1000);
            throw new NotImplementedException("Oops");
        }
    }
}
