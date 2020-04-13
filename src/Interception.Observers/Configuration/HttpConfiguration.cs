namespace Interception.Observers.Configuration
{
    /// <summary>
    /// Configuration for http requests tracking
    /// </summary>
    public class HttpConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "http_metrics";

        /// <summary>
        /// enable tracking
        /// </summary>
        public bool Enabled { get; set; } = true;

        /// <summary>
        /// metric name
        /// </summary>
        public string Name { get; set; } = "http_request";
    }
}
