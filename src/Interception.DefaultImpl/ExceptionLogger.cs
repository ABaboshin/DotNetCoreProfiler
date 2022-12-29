using Interception.Attributes;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System;
using System.Runtime.CompilerServices;

namespace Interception.DefaultImpl
{
    public class ExceptionLogger
    {
        [ExceptionLogger]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void LogException(Exception exception)
        {
            var logged = false;
            if (DependencyInjection.Instance.ServiceProvider != null)
            {
                var logger = DependencyInjection.Instance.ServiceProvider.GetService<ILogger<ExceptionLogger>>();
                if (logger != null)
                {
                    logger.LogWarning(exception, "An error occured by calling the interceptor");
                    logged = true;
                }
            }

            if (!logged)
            {
                Console.WriteLine($"An error occured by calling the interceptor {exception}");
            }
        }

        static ExceptionLogger() { }
    }
}
