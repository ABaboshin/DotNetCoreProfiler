using MassTransit;
using Microsoft.Extensions.Logging;
using Quartz;
using SampleApp.MessageBus;
using System.Threading.Tasks;

namespace SampleApp.ScheduledJobs
{
    public class SampleJob : IJob
    {
        private readonly IBusControl _busControl;
        private readonly ILogger<SampleJob> _logger;

        public SampleJob(IBusControl busControl, ILoggerFactory loggerFactory)
        {
            _busControl = busControl;
            _logger = loggerFactory.CreateLogger<SampleJob>();
        }

        public async Task Execute(IJobExecutionContext context)
        {
            _logger.LogInformation("test");
            await _busControl.Publish(new MyMessage { Id = 3 });
        }
    }
}
