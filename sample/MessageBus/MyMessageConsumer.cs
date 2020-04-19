using MassTransit;
using System;
using System.Threading.Tasks;

namespace SampleApp.MessageBus
{
    public class MyMessageConsumer :
        IConsumer<MyMessage>
    {
        public async Task Consume(ConsumeContext<MyMessage> context)
        {
            Console.WriteLine("MyMessageConsumer.Consume");
            await Task.Delay(3000);
            //throw new NotImplementedByDesignException();
        }
    }
}
