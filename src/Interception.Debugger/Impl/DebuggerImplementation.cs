﻿using dnlib.DotNet;
using Interception.Attributes.Debugger;
using Interception.Core.Info;
using Interception.Tracing.Impl;
using System;
using System.Linq;
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

    public class DebuggerInitializationImplementation
    { 
        [DebuggerInitializer]
        public static void DebuggerInitializer (ProfilerInfo info)
        {
            foreach (var debug in info.Debug)
            {
                ModuleContext modCtx = ModuleDef.CreateModuleContext();
                var module = ModuleDefMD.Load(debug.Dll, modCtx);
                module.LoadEverything();

                var type = module.GetTypes().Where(t => t.FullName == debug.TargetMethod.TypeName).FirstOrDefault();
                if (type is null)
                {
                    continue;
                }

                var method = type.Methods.Where(m =>m.Name == debug.TargetMethod.MethodName && m.GetParamCount() == debug.TargetMethod.MethodParametersCount).FirstOrDefault();
                if (method is null)
                {
                    continue;
                }

                var offset = method.Body.Instructions.ToList().FindIndex(i => i.SequencePoint != null && i.SequencePoint.StartLine == debug.LineNumber && i.SequencePoint.Document.Url.EndsWith(debug.SourceFile));
                if (offset != -1)
                {
                    Interception.Interop.AddDebuggerOffset(debug.TargetMethod, offset);
                    Interception.Interop.AddMethodParameters(debug.TargetMethod, method.Parameters.Select(p => p.Name).ToArray());
                    Interception.Interop.AddMethodVariables(debug.TargetMethod, method.Body.Variables.Select(p => !string.IsNullOrEmpty(p.Name) ? p.Name : $"v{p.Index}").ToArray());
                }
            }

            Interception.Interop.StartDebugger();
        }
    }
}
