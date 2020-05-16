using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Interception.Core.Interop
{
    public static class NativeMethods
    {
        public static void AddInterceptor(ImportInterception intercepion)
        {
            intercepion.Key = $"{intercepion.CallerAssembly}.{intercepion.TargetAssemblyName}.{intercepion.TargetTypeName}.{intercepion.TargetMethodName}.{intercepion.TargetMethodParametersCount}";

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddInterceptor(intercepion);
            }
            else
            {
                NonWindows.AddInterceptor(intercepion);
            }
        }

        public static List<InterceptionInfo> GetInterceptions(string key)
        {
            var ptr = Marshal.AllocHGlobal(4);
            int count = 0;

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.GetInterceptions(out ptr, key, out count);
            }
            else
            {
                NonWindows.GetInterceptions(out ptr, key, out count);
            }

            var interceptions = new List<InterceptionInfo>();

            var size = Marshal.SizeOf(typeof(InterceptionInfoInterop));

            for (int i = 0; i < count; i++)
            {
                var item = Marshal.PtrToStructure<InterceptionInfoInterop>(new IntPtr(ptr.ToInt64() + i * size));
                interceptions.Add(new InterceptionInfo { AssemblyName = Marshal.PtrToStringAnsi(item.AssemblyName), TypeName = Marshal.PtrToStringAnsi(item.TypeName) });
            }

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.FreeGetInterceptionsMemory(out ptr, count);
            }
            else
            {
                NonWindows.FreeGetInterceptionsMemory(out ptr, count);
            }

            Marshal.FreeHGlobal(ptr);

            return interceptions;
        }

        private static class Windows
        {
            [DllImport("DotNetCoreProfiler.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddInterceptor(ImportInterception interception);

            [DllImport("DotNetCoreProfiler.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void GetInterceptions(out IntPtr Pointer, string key, out int size);

            [DllImport("DotNetCoreProfiler.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void FreeGetInterceptionsMemory(out IntPtr Pointer, int size);
        }

        private static class NonWindows
        {
            [DllImport("DotNetCoreProfiler", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddInterceptor(ImportInterception interception);

            [DllImport("DotNetCoreProfiler", CallingConvention = CallingConvention.Cdecl)]
            public static extern void GetInterceptions(out IntPtr Pointer, string key, out int size);

            [DllImport("DotNetCoreProfiler", CallingConvention = CallingConvention.Cdecl)]
            public static extern void FreeGetInterceptionsMemory(out IntPtr Pointer, int size);
        }
    }
}
