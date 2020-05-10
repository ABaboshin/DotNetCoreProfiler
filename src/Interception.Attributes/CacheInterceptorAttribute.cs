using System;

namespace Interception.Attributes
{
    /// <summary>
    /// cache interceptor
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class CacheInterceptorAttribute : Attribute, IInterceptorAttribute
    {
        public Type UserAttributeType { get; } = typeof(CacheAttribute);
    }
}
