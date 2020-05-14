using System;
using System.Runtime.InteropServices;

namespace Interception
{
    public static class NativeMethods
    {
        public static void AddInterceptor(ImportInterception intercepion)
        {
            //Console.WriteLine($"{intercepion.CallerAssembly} {intercepion.TargetAssemblyName} {intercepion.TargetMethodName} {intercepion.TargetTypeName} {intercepion.TargetMethodParametersCount} ");

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
