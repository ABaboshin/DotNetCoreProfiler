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
        public bool ConsumerEnabled { get; set; } = true;

        /// <summary>
        /// consumer metric name
        /// </summary>
        public string ConsumerName { get; set; } = "redis_consumer";
    }
}
