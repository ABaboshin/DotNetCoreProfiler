using System;

namespace Interception.Attributes
{
    /// <summary>
    /// monitor attribute
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class MonitorAttribute : MethodInterceptorAttribute
    {
        /// <summary>
        /// metric name
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// inject parameter values into metrc
        /// </summary>
        public string[] Parameters { get; set; } = new string[] { };

        /// <summary>
        /// inject return value into metric
        /// </summary>
        public bool ReturnValue { get; set; } = false;
    }
}
