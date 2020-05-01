using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class InterceptAttribute : Attribute
    {
        public string CallerAssembly { get; set; }
        public string TargetAssemblyName { get; set; }
        public string TargetTypeName { get; set; }
        public string TargetMethodName { get; set; }
        public int TargetMethodParametersCount { get; set; }
    }
}
