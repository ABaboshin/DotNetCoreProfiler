using System;

namespace Interception.Attributes
{
    /// <summary>
    /// validate parameter attribute
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class ValidationAttribute : Attribute
    {
    }
}
