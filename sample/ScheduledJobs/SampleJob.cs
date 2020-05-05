using MassTransit;
using Quartz;
using SampleApp.MessageBus;
using System.Threading.Tasks;

namespace SampleApp.ScheduledJobs
{
    public class SampleJob : IJob
    {
        private readonly IBusControl _busControl;

        public SampleJob(IBusControl busControl)
        {
            _busControl = busControl;
        }

        public async Task Execute(IJobExecutionContext context)
        {
            await _busControl.Publish(new MyMessage { Id = 3 });
        }
    }
}
