using Interception.Attributes;
using System;

namespace Interception.Core
{
    [MethodInterceptorImplementation(typeof(ParameterValidationAttribute))]
    public class AttributeValidationInterceptor : BaseInterceptor
    {
        protected override void ExecuteAfter(object result, Exception exception)
        {
        }

        protected override void ExecuteBefore()
        {
        }
    }
}
