using OpenTracing;
using OpenTracing.Tag;
using System;

namespace Interception.Tracing.Extensions
{
    public static class ISpanExtensions
    {
        public static ISpan SetException(this ISpan span, Exception ex)
        {
            return span
                .SetTag(Tags.Error, true)
                .SetTag("exceptionType", ex.GetType().FullName)
                .SetTag("exceptionStackTrace", ex.StackTrace)
                .SetTag("exceptionMessage", ex.Message);
        }
    }
}
