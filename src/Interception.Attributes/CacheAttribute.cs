using System;

namespace Interception.Attributes
{
    /// <summary>
    /// cache attribute
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class CacheAttribute : Attribute, IUsageAttribute
    {
        /// <summary>
        /// cache for seconds
        /// </summary>
        public int DurationSeconds { get; set; }

        /// <summary>
        /// cache based on parameters
        /// </summary>
        public string[] Parameters { get; set; }
    }
}
