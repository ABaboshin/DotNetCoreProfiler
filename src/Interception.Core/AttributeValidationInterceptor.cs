using Interception.Attributes;

namespace Interception.Core
{
    [MethodInterceptorImplementation(typeof(ParameterValidationAttribute))]
    public class AttributeValidationInterceptor : BaseInterceptor
    {
        public override int Priority => 1000;
    }
}
