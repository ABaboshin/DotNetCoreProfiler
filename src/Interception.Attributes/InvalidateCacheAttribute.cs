using System;

namespace Interception.Attributes
{
    /// <summary>
    /// invalicate cache
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class InvalidateCacheAttribute : MethodInterceptorAttribute
    {
        /// <summary>
        /// name of the method
        /// which cache has to be invlidated
        /// </summary>
        public string Name { get; set; }
    }
}
