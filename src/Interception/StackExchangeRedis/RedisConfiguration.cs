namespace Interception.StackExchangeRedis
{
    /// <summary>
    /// configuration for redis
    /// </summary>
    public class RedisConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "redis_metrics";

        /// <summary>
        /// enable consumer tracking
        /// </summary>
        public bool Enabled { get; set; } = true;

        /// <summary>
        /// consumer metric name
        /// </summary>
        public string ConsumerName { get; set; } = "redis_consumer";

        /// <summary>
        /// publisher metric name
        /// </summary>
        public string PublisherName { get; set; } = "redis_publisher";
    }
}
