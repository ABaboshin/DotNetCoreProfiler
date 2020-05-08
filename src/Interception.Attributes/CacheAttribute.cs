using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class CacheAttribute : Attribute
    {
        public int DurationSeconds { get; set; }
        public string[] Parameters { get; set; }
    }
}
