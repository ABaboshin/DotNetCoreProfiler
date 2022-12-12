namespace Interception.Observers.Samplers.Configuration
{
    /// <summary>
    /// Configuration for http handler
    /// </summary>
    public class SamplersConfiguration
    {
        /// <summary>
        /// Section name in configuration
        /// </summary>
        public static readonly string SectionKey = "samplers_metrics";

        /// <summary>
        /// enable cpu samples
        /// </summary>
        public bool CPUEnabled { get; set; } = true;

        /// <summary>
        /// enable mem samples
        /// </summary>
        public bool MemEnabled { get; set; } = true;

        /// <summary>
        /// enable gc samples
        /// </summary>
        public bool GCEnabled { get; set; } = true;

        /// <summary>
        /// cpu samples
        /// </summary>
        public string CPUName { get; set; } = "cpu";

        /// <summary>
        /// mem samples
        /// </summary>
        public string MemName { get; set; } = "mem";

        /// <summary>
        /// gc samples
        /// </summary>
        public string GCName { get; set; } = "gc";

        /// <summary>
        /// cpu samples interval
        /// </summary>
        public int CPUInterval { get; set; } = 60;

        /// <summary>
        /// mem samples
        /// </summary>
        public int MemInterval { get; set; } = 60;

        /// <summary>
        /// gc samples
        /// </summary>
        public int GCInterval { get; set; } = 60;
    }
}
