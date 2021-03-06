﻿using System;

namespace Interception.OpenTracing.Statsd.Extensions
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

        public static string EscapeTagName(this string value)
        {
            return value.Replace("-", "_").Replace(".", "_");
        }
    }
}
