using System.Collections.Generic;

namespace Interception.Core.Info
{
    public class StrictInterceptionInfo
    {
        public TypeInfo Interceptor { get; set; }
        public TargetMethodnfo Target { get; set; }
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

    public class TargetMethodnfo
    {
        public string AssemblyName { get; set; }
        public string MethodName { get; set; }
        public int MethodParametersCount { get; set; }
        public string TypeName { get; set; }
    }

    public class TraceMethodInfo
    {
        public TargetMethodnfo TargetMethod { get; set; }
        public List<string> Parameters { get; set; }
        public string Name { get; set; }
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
        public List<TraceMethodInfo> Traces { get; set; }
        public string Path { get; set; }
    }
}
