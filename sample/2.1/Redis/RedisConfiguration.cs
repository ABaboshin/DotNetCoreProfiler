namespace SampleApp.Redis
{
    public class RedisConfiguration
    {
        public static readonly string SectionKey = "redis";
        public string ConnectionString { get; set; }
    }
}
