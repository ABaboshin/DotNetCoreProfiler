using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Common
{
    /// <summary>
    /// tags extensions
    /// </summary>
    internal static class TagValuesExtensions
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
