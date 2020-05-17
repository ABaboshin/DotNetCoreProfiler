using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Interception.Core.Interop
{
    public static class NativeMethods
    {
        public static void AddInterceptor(ImportInterception intercepion)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddInterceptor(intercepion);
            }
            else
            {
                NonWindows.AddInterceptor(intercepion);
            }
        }

        private static class Windows
        {
            [DllImport("DotNetCoreProfiler.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddInterceptor(ImportInterception interception);
        }

        private static class NonWindows
        {
            [DllImport("DotNetCoreProfiler", CallingConvention = CallingConvention.Cdecl)]
            public static extern void AddInterceptor(ImportInterception interception);
        }
    }
}
