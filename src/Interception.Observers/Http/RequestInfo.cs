using System;

namespace Interception.Observers.Http
{
    /// <summary>
    /// request info
    /// </summary>
    internal class RequestInfo
    {
        /// <summary>
        /// start time
        /// </summary>
        public DateTime Start { get; set; }

        /// <summary>
        /// controller name
        /// </summary>
        public string ControllerName { get; set; }

        /// <summary>
        /// action name
        /// </summary>
        public string ActionName { get; set; }

        /// <summary>
        /// trace identifier
        /// </summary>
        public string TraceIdentifier { get; set; }

        /// <summary>
        /// exception if an error
        /// </summary>
        public Exception Exception { get; internal set; }
    }
}
