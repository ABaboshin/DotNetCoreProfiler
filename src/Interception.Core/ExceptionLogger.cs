using Interception.Attributes;
using System;
using System.Runtime.CompilerServices;

namespace Interception.Core
{
    public class ExceptionLogger
    {
        [ExceptionLogger]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void LogException(Exception exception) { Console.WriteLine("From interceptor " + exception.ToString()); }

        static ExceptionLogger() { }
    }
}
