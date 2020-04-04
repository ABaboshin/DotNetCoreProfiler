using MassTransit;
using System.Threading.Tasks;

namespace SampleApp.MessageBus
{
    public class MyMessageConsumer :
        IConsumer<MyMessage>
    {
        public async Task Consume(ConsumeContext<MyMessage> context)
        {
            await Task.Delay(1000);
            throw new NotImplementedByDesignException();
        }
    }
}
