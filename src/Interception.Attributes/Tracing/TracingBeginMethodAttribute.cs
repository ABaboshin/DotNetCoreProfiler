using System;

namespace Interception.Attributes.Tracing
{
    /// <summary>
    /// begin tracing span
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class TracingBeginMethodAttribute : Attribute
    {
    }
}
