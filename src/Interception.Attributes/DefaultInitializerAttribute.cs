using System;

namespace Interception.Attributes
{
    /// <summary>
    /// a default attribute
    /// to create a default(T)
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class DefaultInitializerAttribute : Attribute
    {
    }
}
