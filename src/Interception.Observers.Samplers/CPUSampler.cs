
using Interception.Observers.Samplers.Configuration;
using Microsoft.Extensions.Configuration;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Timers;

namespace Interception.Observers.Samplers
{
    public class CPUSampler
    {
        private readonly SamplersConfiguration _configuration;
        private readonly Timer _timer;

        public CPUSampler(SamplersConfiguration configuration)
        {
            _configuration = configuration;

            if (_configuration.CPUEnabled)
            {
                Console.WriteLine("!!!!!!!!!!!!!!!!! CPU ENABLED");
                _timer = new Timer(_configuration.CPUInterval * 1000);
                _timer.Elapsed += _timer_Elapsed;
                _timer.Enabled = true;
                _timer.AutoReset = true;
                _timer.Start();
            }
        }

        private void _timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            Console.WriteLine("!!!!!!!!!!!!!!!!! CPU elapsed");
            GlobalTracer.Instance
                    .BuildSpan(_configuration.CPUName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient)
                    .WithTag("processors_count", System.Environment.ProcessorCount)
                    .WithTag("user_processor_time", System.Diagnostics.Process.GetCurrentProcess().UserProcessorTime.ToString())
                    .Start()
                    .Finish();
        }

        public static void ConfigureAndStart ()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var cpuSamplerConfiguration = configuration.GetSection(SamplersConfiguration.SectionKey).Get<SamplersConfiguration>();
            new CPUSampler(cpuSamplerConfiguration);
        }
    }
}
