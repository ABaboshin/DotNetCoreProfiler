using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public sealed class MethodInterceptorImplementationAttribute : Attribute
    {
        public Type MethodInterceptorAttribute { get; set; }

        public MethodInterceptorImplementationAttribute(Type methodInterceptorAttribute)
        {
            MethodInterceptorAttribute = methodInterceptorAttribute;
        }
    }
}
