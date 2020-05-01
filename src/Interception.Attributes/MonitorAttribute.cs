using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class MonitorAttribute : Attribute
    {
    }
}
