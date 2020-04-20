using System;

namespace Interception.OpenTracing.Prometheus.Extensions
{
    /// <summary>
    /// tags extensions
    /// </summary>
    public static class TagValuesExtensions
    {
        /// <summary>
        /// Escape tags values for statsd
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public static string EscapeTagValue(this string value)
        {
            return value.Replace(Environment.NewLine, " ").Replace(",", " ");
        }
    }
}
