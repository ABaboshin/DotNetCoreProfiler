namespace Interception.Tracing
{
    public class TracingConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "tracing";

        public string Collector { get; set; }
    }
}
