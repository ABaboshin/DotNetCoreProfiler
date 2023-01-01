using System;

namespace Interception.Attributes
{
    /// <summary>
    /// an exception logger attribute
    /// to log before/after exception
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class ExceptionLoggerAttribute : Attribute
    {
    }

}
