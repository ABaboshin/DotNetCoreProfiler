namespace Interception.Observers.Configuration
{
    /// <summary>
    /// Configuration for aspnetcore
    /// </summary>
    public class AspNetCoreConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "aspnetcore_metrics";

        /// <summary>
        /// enable tracking
        /// </summary>
        public bool Enabled { get; set; } = true;

        /// <summary>
        /// metric name
        /// </summary>
        public string Name { get; set; } = "incoming";
    }
}
