using System;
using System.Collections.Generic;

namespace Interception.Metrics.Extensions
{
    /// <summary>
    /// exception extensions
    /// </summary>
    public static class ExceptionExtensions
    {
        /// <summary>
        /// extract tag values for metrics from a given exception
        /// </summary>
        /// <param name="exception">exception</param>
        /// <returns>tags</returns>
        public static IEnumerable<string> GetTags(this Exception exception)
        {
            return new[] {
                $"exceptionType:{exception.GetType().FullName}",
                $"exceptionStackTrace:{exception.StackTrace.EscapeTagValue()}",
                $"exceptionMessage:{exception.Message.EscapeTagValue()}"
            };
        }
    }
}
