using System.Collections.Generic;

namespace Interception.Core.Info
{
    public class StrictInterceptionInfo
    {
        public TypeInfo Interceptor { get; set; }
        public TargetMethod Target { get; set; }
        public string AssemblyPath { get; set; }
        public int Priority { get; set; }
    }

    public class LoaderInfo
    {
        public string TypeName { get; set; }
        public string AssemblyPath { get; set; }
    }

    public class DefaultInitializerInfo
    {
        public string MethodName { get; set; }
        public string TypeName { get; set; }
        public string AssemblyPath { get; set; }
        public string AssemblyName { get; set; }
    }

    public class ExceptionLoggerInfo
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
    }

    public class TargetMethod
    {
        public string AssemblyName { get; set; }
        public string MethodName { get; set; }
        public int MethodParametersCount { get; set; }
        public string TypeName { get; set; }
    }

    public class ProfilerInfo
    {
        public LoaderInfo Loader { get; set; }
        public List<string> SkipAssemblies { get; set; }
        public List<StrictInterceptionInfo> Strict { get; set; }
        public DefaultInitializerInfo DefaultInitializer { get; set; }
        public ExceptionLoggerInfo ExceptionLogger { get; set; }
        public string Path { get; set; }
    }
}
