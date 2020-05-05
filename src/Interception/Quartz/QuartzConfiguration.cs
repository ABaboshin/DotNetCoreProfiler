namespace Interception.Quartz
{
    /// <summary>
    /// configuration for quartz
    /// </summary>
    public class QuartzConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "quartz_metrics";

        /// <summary>
        /// enable publisher tracking
        /// </summary>
        public bool Enabled { get; set; } = true;

        /// <summary>
        /// publisher metric name
        /// </summary>
        public string Name { get; set; } = "scheduled_job";
    }
}
