using System.Runtime.InteropServices;

namespace Interception.Core.Interop
{
    [StructLayout(LayoutKind.Sequential)]
    public struct ImportInterception
    {
        public string CallerAssembly;
        public string TargetAssemblyName;
        public string TargetMethodName;
        public string TargetTypeName;
        public int TargetMethodParametersCount;
        public string InterceptorTypeName;
        public string InterceptorAssemblyName;
        public bool IsComposed;
    }
}
