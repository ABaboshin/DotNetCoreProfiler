namespace Interception.Cache
{
    /// <summary>
    /// cache configuration
    /// </summary>
    public class CacheConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "cache";

        public string Type { get; set; }

        public string Configuration { get; set; }
    }
}
