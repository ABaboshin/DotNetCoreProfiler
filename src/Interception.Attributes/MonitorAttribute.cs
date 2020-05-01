using System;
using System.Collections.Generic;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class MonitorAttribute : Attribute
    {
        public string Name { get; set; }
        public bool MonitorReturnValue { get; set; }
        public string[] MonitorParameters { get; set; }
    }
}
