using System;

namespace Interception
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class InterceptAttribute : Attribute
    {
        public string CallerAssembly { get; set; }
        public string TargetAssemblyName { get; set; }
        public string TargetTypeName { get; set; }
        public string TargetMethodName { get; set; }
        public int TargetMethodParametersCount { get; set; }
    }
}
