using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Common
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
    public class InitializeAttribute : Attribute
    {
    }
}
