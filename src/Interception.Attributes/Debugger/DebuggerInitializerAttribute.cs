using System;

namespace Interception.Attributes.Debugger
{
    /// <summary>
    /// Debugger initializer
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class DebuggerInitializerAttribute : Attribute
    {
    }
}
