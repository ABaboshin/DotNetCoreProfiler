using System;

namespace Interception.Attributes.Debugger
{
    /// <summary>
    /// end Debugger span
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class DebuggerEndMethodAttribute : Attribute
    {
    }
}
