using System;

namespace Interception.Attributes
{
    /// <summary>
    /// implemention for the attribute defined interceptor
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public sealed class MethodInterceptorImplementationAttribute : Attribute
    {
        /// <summary>
        /// attribute type
        /// </summary>
        public Type MethodInterceptorAttribute { get; set; }

        public MethodInterceptorImplementationAttribute(Type methodInterceptorAttribute)
        {
            MethodInterceptorAttribute = methodInterceptorAttribute;
        }
    }
}
