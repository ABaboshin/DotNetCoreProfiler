using System;

namespace Interception.Attributes
{
    /// <summary>
    /// invalicate cache
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class InvalidateCacheAttribute : MethodInterceptorAttribute
    {
        public string Name { get; set; }
    }
}
