using System.Runtime.InteropServices;
using System.Threading;

namespace Interception.Core.Interop
{
    public class Profiler
    {
        public static void RejitAll()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Windows.RejitAll();
            }

            NonWindows.RejitAll();

            Thread.Sleep(1000);
        }

        private static class Windows
        {
            [DllImport("DotNetCoreProfiler.dll")]
            public static extern void RejitAll();
        }

        private static class NonWindows
        {
            [DllImport("DotNetCoreProfiler")]
            public static extern void RejitAll();
        }
    }
}
