using MassTransit.Configuration;

namespace Interception.MassTransit
{
    public class OpenTracingPublishPipeSpecificationObserver : IPublishPipeSpecificationObserver
    {
        public void MessageSpecificationCreated<TMessage>(IMessagePublishPipeSpecification<TMessage> specification)
                where TMessage : class
        {
            specification.AddPipeSpecification(new OpenTracingPipeSpecification<TMessage>());
        }
    }
}