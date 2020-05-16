using System;
using System.Runtime.InteropServices;

namespace Interception.Core.Interop
{
    [StructLayout(LayoutKind.Sequential)]
    public struct InterceptionInfoInterop
    {
        public IntPtr AssemblyName;
        public IntPtr TypeName;
    }
}
