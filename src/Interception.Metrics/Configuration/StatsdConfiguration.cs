﻿using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Metrics.Configuration
{
    /// <summary>
    /// Statsd configuration
    /// </summary>
    public class StatsdConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "statsd";

        /// <summary>
        /// statsd server
        /// </summary>
        public string Server { get; set; }

        /// <summary>
        /// statsd port
        /// </summary>
        public int Port { get; set; }
    }
}