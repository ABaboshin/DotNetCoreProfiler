using Interception.Core.Info;
using System;
using System.Runtime.InteropServices;

namespace Interception
{
    public class Interop
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct _TargetMethodInfo
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public string AssemblyName;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string TypeName;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string MethodName;
            public int MethodParametersCount;
        }

        public static void AddMethodParameters(TargetMethodInfo methodInfo, string[] parameters)
        {
            var mi = new _TargetMethodInfo
            {
                AssemblyName = methodInfo.AssemblyName,
                MethodName = methodInfo.MethodName,
                TypeName = methodInfo.TypeName,
                MethodParametersCount = methodInfo.MethodParametersCount,
            };

            var s_array = new IntPtr[parameters.Length];
            for (int i = 0; i < parameters.Length; ++i)
            {
                s_array[i] = Marshal.StringToHGlobalUni(parameters[i]);
            }
            var gH = GCHandle.Alloc(s_array, GCHandleType.Pinned);

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddMethodParameters(mi, gH.AddrOfPinnedObject(), s_array.Length);
            }
            else
            {
                NonWindows.AddMethodParameters(mi, gH.AddrOfPinnedObject(), s_array.Length);
            }

            gH.Free();
            for (int i = 0; i < parameters.Length; ++i)
            {
                Marshal.FreeHGlobal(s_array[i]);
            }
        }

        public static void AddMethodVariables(TargetMethodInfo methodInfo, string[] variables)
        {
            var mi = new _TargetMethodInfo
            {
                AssemblyName = methodInfo.AssemblyName,
                MethodName = methodInfo.MethodName,
                TypeName = methodInfo.TypeName,
                MethodParametersCount = methodInfo.MethodParametersCount,
            };

            var s_array = new IntPtr[variables.Length];
            for (int i = 0; i < variables.Length; ++i)
            {
                s_array[i] = Marshal.StringToHGlobalUni(variables[i]);
            }
            var gH = GCHandle.Alloc(s_array, GCHandleType.Pinned);

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddMethodVariables(mi, gH.AddrOfPinnedObject(), s_array.Length);
            }
            else
            {
                NonWindows.AddMethodVariables(mi, gH.AddrOfPinnedObject(), s_array.Length);
            }

            gH.Free();
            for (int i = 0; i < variables.Length; ++i)
            {
                Marshal.FreeHGlobal(s_array[i]);
            }
        }

        public static void AddDebuggerOffset(TargetMethodInfo methodInfo, int offset)
        {
            var mi = new _TargetMethodInfo
            {
                AssemblyName = methodInfo.AssemblyName,
                MethodName = methodInfo.MethodName,
                TypeName = methodInfo.TypeName,
                MethodParametersCount = methodInfo.MethodParametersCount,
            };

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddDebuggerOffset(mi, offset);
            }
            else
            {
                NonWindows.AddDebuggerOffset(mi, offset);
            }
        }

        public static void StartDebugger()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.StartDebugger();
            }
            else
            {
                NonWindows.StartDebugger();
            }
        }

        private static class Windows
        {
            [DllImport("DotNetCoreProfiler.dll")]
            public static extern void AddMethodParameters([In] _TargetMethodInfo methodInfo, [In] IntPtr parameters, [In] int parametersLength);

            [DllImport("DotNetCoreProfiler.dll")]
            public static extern void AddMethodVariables([In] _TargetMethodInfo methodInfo, [In] IntPtr variables, [In] int variablesLength);

            [DllImport("DotNetCoreProfiler.dll")]
            public static extern void AddDebuggerOffset([In] _TargetMethodInfo methodInfo, [In] int offset);
            [DllImport("DotNetCoreProfiler.dll")]
            public static extern void StartDebugger();
        }

        private static class NonWindows
        {
            [DllImport("DotNetCoreProfiler")]
            public static extern void AddMethodParameters([In] _TargetMethodInfo methodInfo, [In] IntPtr parameters, [In] int parametersLength);

            [DllImport("DotNetCoreProfiler")]
            public static extern void AddMethodVariables([In] _TargetMethodInfo methodInfo, [In] IntPtr variables, [In] int variablesLength);

            [DllImport("DotNetCoreProfiler")]
            public static extern void AddDebuggerOffset([In] _TargetMethodInfo methodInfo, [In] int offset);
            [DllImport("DotNetCoreProfiler")]
            public static extern void StartDebugger();
        }
    }
}
