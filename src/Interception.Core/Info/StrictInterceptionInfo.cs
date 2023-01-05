using System.Collections.Generic;

namespace Interception.Core.Info
{
    public class StrictInterceptionInfo
    {
        public TypeInfo Interceptor { get; set; }
        public TargetMethodInfo TargetMethod { get; set; }
        public int Priority { get; set; }
    }

    public class InterceptorMethodInfo
    {
        public string MethodName { get; set; }
        public string TypeName { get; set; }
        public string AssemblyPath { get; set; }
        public string AssemblyName { get; set; }
    }

    public class TypeInfo
    {
        public string AssemblyName { get; set; }
        public string TypeName { get; set; }
        public string AssemblyPath { get; set; }
    }

    public class TargetMethodInfo
    {
        public string AssemblyName { get; set; }
        public string MethodName { get; set; }
        public int MethodParametersCount { get; set; }
        public string TypeName { get; set; }
    }

    public class TraceMethodInfo
    {
        public TargetMethodInfo TargetMethod { get; set; }
        public List<string> Parameters { get; set; }
        public string Name { get; set; }
    }

    public class DebugMethodInfo
    {
        public TargetMethodInfo TargetMethod { get; set; }
        public string Dll { get; set; }
        public string SourceFile { get; set; }
        public int LineNumber { get; set; }
    }

    public class ProfilerInfo
    {
        public TypeInfo Loader { get; set; }
        public List<string> SkipAssemblies { get; set; }
        public List<StrictInterceptionInfo> Strict { get; set; }
        public InterceptorMethodInfo DefaultInitializer { get; set; }
        public InterceptorMethodInfo ExceptionLogger { get; set; }
        public InterceptorMethodInfo TracingBeginMethod { get; set; }
        public InterceptorMethodInfo TracingEndMethod { get; set; }
        public InterceptorMethodInfo TracingAddParameterMethod { get; set; }
        public InterceptorMethodInfo DebuggerBeginMethod { get; set; }
        public InterceptorMethodInfo DebuggerEndMethod { get; set; }
        public InterceptorMethodInfo DebuggerAddParameterMethod { get; set; }
        public InterceptorMethodInfo DebuggerInitializerMethod { get; set; }
        public List<TraceMethodInfo> Traces { get; set; }
        public List<DebugMethodInfo> Debug { get; set; }
        public string Path { get; set; }
    }
}
