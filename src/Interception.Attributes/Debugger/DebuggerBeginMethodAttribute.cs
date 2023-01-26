using System;

namespace Interception.Attributes.Debugger
{
    /// <summary>
    /// begin Debugger span
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class DebuggerBeginMethodAttribute : Attribute
    {
    }
}
