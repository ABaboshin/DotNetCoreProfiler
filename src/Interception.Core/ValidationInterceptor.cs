using Interception.Attributes;

namespace Interception.Core
{
    [MethodInterceptorImplementation(typeof(ValidationAttribute))]
    public class ValidationInterceptor : BaseInterceptor
    {
        public override int Priority => 100;
    }
}
