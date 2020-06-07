using System;

namespace Interception.Attributes.Validation
{
    /// <summary>
    /// validate parameter attribute
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class ValidationAttribute : Attribute
    {
    }
}
