using System;
using System.Runtime.InteropServices;

namespace Interception
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
        public string InterceptorMethodName;
        public string InterceptorAssemblyName;
        public IntPtr Signature;
        public int SignatureLength;
    }
}
