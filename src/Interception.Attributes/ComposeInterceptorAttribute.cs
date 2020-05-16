using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class ComposeInterceptorAttribute : Attribute
    {
    }
}
