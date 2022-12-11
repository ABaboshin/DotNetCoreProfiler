using Interception.Observers.Samplers.Configuration;
using Microsoft.Extensions.Configuration;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Timers;

namespace Interception.Observers.Samplers
{
    public class GCSampler
    {
        private readonly SamplersConfiguration _configuration;
        private readonly Timer _timer;
        private readonly GCEventListener _listener;

        public GCSampler(SamplersConfiguration configuration)
        {
            _configuration = configuration;

            if (_configuration.GCEnabled)
            {
                Console.WriteLine("!!!!!!!!!!!!!!!!! gc ENABLED");
                _timer = new Timer(_configuration.GCInterval * 1000);
                _timer.Elapsed += _timer_Elapsed;
                _timer.Enabled = true;
                _timer.AutoReset = true;
                _timer.Start();
                
                _listener = new GCEventListener();
            }
        }

        private void _timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            var sample = _listener.Sample();
            var span = GlobalTracer.Instance
                    .BuildSpan(_configuration.GCName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient);

            foreach (var item in sample)
            {
                span = span
                    .WithTag(item.Key, item.Value.ToString());
            }
            
            span 
                    .Start()
                    .Finish();
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var gcSamplerConfiguration = configuration.GetSection(SamplersConfiguration.SectionKey).Get<SamplersConfiguration>();
            new GCSampler(gcSamplerConfiguration);
            
        }
    }
}
