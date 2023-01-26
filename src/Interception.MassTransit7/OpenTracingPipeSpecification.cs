using GreenPipes;
using MassTransit;
using MassTransit.Configuration;
using System.Collections.Generic;
using System.Linq;

namespace Interception.MassTransit
{
    public class OpenTracingPipeSpecification<TMessage> : IPipeSpecification<PublishContext<TMessage>> where TMessage : class
    {
        public void Apply(IPipeBuilder<PublishContext<TMessage>> builder)
        {
            builder.AddFilter(new OpenTracingPublishMessageFilter<TMessage>());
        }

        public IEnumerable<ValidationResult> Validate()
        {
            return Enumerable.Empty<ValidationResult>();
        }
    }
}