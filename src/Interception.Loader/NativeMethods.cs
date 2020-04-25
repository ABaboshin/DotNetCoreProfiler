using System.Runtime.InteropServices;

namespace Interception
{
    public static class NativeMethods
    {
        public static void AddInterceptor(ImportInterception interception)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.AddInterceptor(interception);
            }
            else
            {
                NonWindows.AddInterceptor(interception);
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
