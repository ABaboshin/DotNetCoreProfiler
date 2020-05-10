using System;

namespace Interception.Attributes
{
    /// <summary>
    /// a strict interceptor 
    /// to intercept an exact method
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class StrictInterceptAttribute : Attribute
    {
        public string CallerAssembly { get; set; }
        public string TargetAssemblyName { get; set; }
        public string TargetTypeName { get; set; }
        public string TargetMethodName { get; set; }
        public int TargetMethodParametersCount { get; set; }
    }
}
