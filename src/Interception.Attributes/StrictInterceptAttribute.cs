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
        /// <summary>
        /// ignore calls to the method from the listed assemblies
        /// </summary>
        // public string[] IgnoreCallerAssemblies { get; set; }

        /// <summary>
        /// assembly name where the method is placed
        /// </summary>
        public string TargetAssemblyName { get; set; }

        /// <summary>
        /// assembly name whom the method is belonging
        /// </summary>
        public string TargetTypeName { get; set; }

        /// <summary>
        /// method name to intercept
        /// </summary>
        public string TargetMethodName { get; set; }

        /// <summary>
        /// count of parameters of the intercepted method
        /// </summary>
        public int TargetMethodParametersCount { get; set; }
    }
}
