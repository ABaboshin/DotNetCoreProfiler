using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Attributes
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class MonitoringInterceptAttribute : Attribute
    {
    }
}
