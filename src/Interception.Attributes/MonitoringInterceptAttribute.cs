using System;

namespace Interception.Attributes
{
    /// <summary>
    /// monitor interceptor
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class MonitoringInterceptAttribute : Attribute, IInterceptorAttribute
    {
        public Type UserAttributeType { get; } = typeof(MonitorAttribute);
    }
}
