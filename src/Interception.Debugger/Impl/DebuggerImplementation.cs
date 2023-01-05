using Interception.Attributes.Debugger;
using Interception.Tracing.Impl;
using System;
using System.Threading;
using System.Xml.Linq;

namespace Interception.Debugger.Impl
{
    public class DebuggerImplementation
    {
        protected static AsyncLocal<OpenTracing.IScope> _scope = new AsyncLocal<OpenTracing.IScope>();

        [DebuggerBeginMethod]
        public static void BeginDebugging(string name)
        {
            Console.WriteLine($"BeginDebugging {name}");
            TracingImplementation.BeginTracing(name);
        }

        [DebuggerAddParameterMethod]
        public static void AddParameter<T>(string type, string name, T value)
        {
            Console.WriteLine($"AddParameter {type} {name} {value}");
            TracingImplementation.AddParameter($"{type}_{name}", value);
        }

        [DebuggerEndMethod]
        public static void EndDebugging()
        {
            Console.WriteLine($"EndDebugging");
            TracingImplementation.EndTracing<object>(null, null);
        }
    }
}
