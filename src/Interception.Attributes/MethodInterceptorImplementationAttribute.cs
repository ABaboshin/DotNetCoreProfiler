using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class MethodInterceptorImplementationAttribute : Attribute, IMethodInterceptorImplementationAttribute
    {
        public Type MethodInterceptorAttribute { get; set; }

        public MethodInterceptorImplementationAttribute(Type methodInterceptorAttribute)
        {
            MethodInterceptorAttribute = methodInterceptorAttribute;
        }
    }
}
