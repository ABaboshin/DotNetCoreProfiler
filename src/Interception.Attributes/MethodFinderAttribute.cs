using System;

namespace Interception.Attributes
{
    /// <summary>
    /// a method finder
    /// to find an exact method
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class MethodFinderAttribute : Attribute
    {
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
