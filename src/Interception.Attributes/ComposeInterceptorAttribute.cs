using System;

namespace Interception.Attributes
{
    /// <summary>
    /// compose interceptor
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class ComposeInterceptorAttribute : Attribute
    {
    }
}
