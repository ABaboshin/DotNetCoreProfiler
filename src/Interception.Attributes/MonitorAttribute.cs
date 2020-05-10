﻿using System;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public class MonitorAttribute : Attribute
    {
        public string Name { get; set; }
        public string[] Parameters { get; set; } = new string[] { };
        public bool ReturnValue { get; set; }
    }
}
