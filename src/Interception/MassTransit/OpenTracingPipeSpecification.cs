using GreenPipes;
using MassTransit;
using System.Collections.Generic;
using System.Linq;

namespace Interception.MassTransit
{
    public class OpenTracingPipeSpecification : IPipeSpecification<PublishContext>
    {
        public IEnumerable<ValidationResult> Validate()
        {
            return Enumerable.Empty<ValidationResult>();
        }

        public void Apply(IPipeBuilder<PublishContext> builder)
        {
            builder.AddFilter(new OpenTracingPublishFilter());
        }
    }
}