namespace Interception.Generator
{
    public class StrictInterception
    {
        public string CallerAssembly { get; set; }
        public TargetMethod Target { get; set; }
        public Interceptor Interceptor { get; set; }
    }

    public class Interceptor
    {
        public string TypeName { get; set; }
        public string AssemblyName { get; set; }
    }

    public class TargetMethod
    {
        public string AssemblyName { get; set; }
        public string MethodName { get; set; }
        public string TypeName { get; set; }
        public int MethodParametersCount { get; set; }
    }

    public class AttributedInterceptor
    {
        public string AttributeType { get; set; }
        public Interceptor Interceptor { get; set; }
    }
}
