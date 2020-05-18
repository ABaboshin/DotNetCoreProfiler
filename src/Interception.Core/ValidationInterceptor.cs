using Interception.Attributes;

namespace Interception.Core
{
    [MethodInterceptorImplementation(typeof(ValidationAttribute))]
    public class ValidationInterceptor : BaseInterceptor
    {
    }
}
