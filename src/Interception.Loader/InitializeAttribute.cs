using System;

namespace Interception
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class InitializeAttribute : Attribute
    {
    }
}
