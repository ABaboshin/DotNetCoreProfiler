
using Interception.Observers.Samplers.Configuration;
using Microsoft.Extensions.Configuration;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Timers;

namespace Interception.Observers.Samplers
{
    public class MemSampler
    {
        private readonly SamplersConfiguration _configuration;
        private readonly Timer _timer;

        public MemSampler(SamplersConfiguration configuration)
        {
            _configuration = configuration;

            if (_configuration.MemEnabled)
            {
                _timer = new Timer(_configuration.MemInterval * 1000);
                _timer.Elapsed += _timer_Elapsed;
                _timer.Enabled = true;
                _timer.AutoReset = true;
                _timer.Start();
            }
        }

        private void _timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            GlobalTracer.Instance
                    .BuildSpan(_configuration.MemName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient)
                    .WithTag("working_set", System.Diagnostics.Process.GetCurrentProcess().WorkingSet64)
                    .WithTag("private_memory_size", System.Diagnostics.Process.GetCurrentProcess().PrivateMemorySize64)
                    .Start()
                    .Finish();
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var memSamplerConfiguration = configuration.GetSection(SamplersConfiguration.SectionKey).Get<SamplersConfiguration>();
            new MemSampler(memSamplerConfiguration);
        }
    }
}
