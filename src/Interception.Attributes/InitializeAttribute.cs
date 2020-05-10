using System;

namespace Interception.Attributes
{
    /// <summary>
    /// asembly initializer
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class InitializeAttribute : Attribute
    {
    }
}
