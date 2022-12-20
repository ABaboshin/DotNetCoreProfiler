namespace Interception.Generator
{
    public class StrictInterceptionInfo
    {
        // public string[] IgnoreCallerAssemblies { get; set; }
        public TypeInfo Interceptor { get; set; }
        public TargetMethod Target { get; set; }
    }

    public class MethodFinderInfo
    {
        public TypeInfo MethodFinder { get; set; }
        public TargetMethod Target { get; set; }
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

    public class AttributedInterceptor
    {
        public string AttributeType { get; set; }
        public TypeInfo Interceptor { get; set; }
    }
}
