using System;
using dnlib.DotNet;
using Interception.Attributes.Debugger;
using Interception.Core.Info;
using System.Linq;

namespace Interception.Debugger.Impl
{
    public class DebuggerInitializationImplementation
    { 
        [DebuggerInitializer]
        public static void DebuggerInitializer (ProfilerInfo info)
        {
            if (info.Debug is null)
            {
                return;
            }
            
            foreach (var debug in info.Debug)
            {
                Console.WriteLine($"Try to add debug for {debug.TargetMethod.AssemblyName}/{debug.TargetMethod.TypeName}.{debug.TargetMethod.MethodName} at {debug.SourceFile}:{debug.LineNumber} from {debug.Dll}");
                try
                {
                    ModuleContext modCtx = ModuleDef.CreateModuleContext();
                    var module = ModuleDefMD.Load(debug.Dll, modCtx);
                    module.LoadEverything();

                    var type = module.GetTypes().Where(t => t.FullName == debug.TargetMethod.TypeName).FirstOrDefault();
                    if (type is null)
                    {
                        Console.WriteLine($"Type {debug.TargetMethod.AssemblyName}/{debug.TargetMethod.TypeName} not found");
                        continue;
                    }

                    var method = type.Methods.Where(m =>m.Name == debug.TargetMethod.MethodName && m.GetParamCount() == debug.TargetMethod.MethodParametersCount).FirstOrDefault();
                    if (method is null)
                    {
                        Console.WriteLine($"Method {debug.TargetMethod.AssemblyName}/{debug.TargetMethod.TypeName}.{debug.TargetMethod.MethodName} not found");
                        continue;
                    }

                    var offset = method.Body.Instructions.ToList().FindIndex(i => i.SequencePoint != null && i.SequencePoint.StartLine == debug.LineNumber && i.SequencePoint.Document.Url.EndsWith(debug.SourceFile));
                    if (offset != -1)
                    {
                        Console.WriteLine($"Try to add debug for {debug.TargetMethod.AssemblyName}/{debug.TargetMethod.TypeName}.{debug.TargetMethod.MethodName} at {debug.SourceFile}:{debug.LineNumber} from {debug.Dll} offset {offset} found");
                        Interop.AddDebuggerOffset(debug.TargetMethod, offset);
                        Interop.AddMethodParameters(debug.TargetMethod, method.Parameters.Select(p => p.Name).ToArray());
                        Interop.AddMethodVariables(debug.TargetMethod, method.Body.Variables.Select(p => !string.IsNullOrEmpty(p.Name) ? p.Name : $"v{p.Index}").ToArray());
                    }
                    else
                    {
                        Console.WriteLine($"Try to add debug for {debug.TargetMethod.AssemblyName}/{debug.TargetMethod.TypeName}.{debug.TargetMethod.MethodName} at {debug.SourceFile}:{debug.LineNumber} from {debug.Dll} offset not found");
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }

            Interception.Interop.StartDebugger();
        }
    }
}
