namespace Interception.Observers.Configuration
{
    /// <summary>
    /// Configuration for http handler
    /// </summary>
    public class HttpHandlerConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "httphandler_metrics";

        /// <summary>
        /// enable tracking
        /// </summary>
        public bool Enabled { get; set; } = true;

        /// <summary>
        /// metric name
        /// </summary>
        public string Name { get; set; } = "outgoing";
    }
}
