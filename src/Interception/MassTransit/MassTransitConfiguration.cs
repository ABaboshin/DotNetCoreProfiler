namespace Interception.MassTransit
{
    /// <summary>
    /// Configuration for masstransit
    /// </summary>
    public class MassTransitConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "masstransit_metrics";

        /// <summary>
        /// enable publisher tracking
        /// </summary>
        public bool PublisherEnabled { get; set; } = true;

        /// <summary>
        /// publisher metric name
        /// </summary>
        public string PublisherName { get; set; } = "masstransit_publish";

        /// <summary>
        /// enable consumer tracking
        /// </summary>
        public bool ConsumerEnabled { get; set; } = true;

        /// <summary>
        /// consumer metric name
        /// </summary>
        public string ConsumerName { get; set; } = "masstransit_consumer";

        public override string ToString()
        {
            return $"PublisherEnabled {PublisherEnabled} PublisherName {PublisherName} ConsumerEnabled {ConsumerEnabled} ConsumerName {ConsumerName}";
        }
    }
}
