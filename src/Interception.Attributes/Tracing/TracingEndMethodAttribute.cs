using System;

namespace Interception.Attributes.Tracing
{
    /// <summary>
    /// end tracing span
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class TracingEndMethodAttribute : Attribute
    {
    }
}
