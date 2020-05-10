using Microsoft.Extensions.Hosting;
using Quartz;
using Quartz.Spi;
using System.Threading;
using System.Threading.Tasks;

namespace SampleApp.ScheduledJobs
{
    public class SchedulerService : BackgroundService
    {
        private readonly ISchedulerFactory _schedulerFactory;
        private readonly IJobFactory _jobFactory;

        public IScheduler Scheduler { get; set; }

        public SchedulerService(ISchedulerFactory schedulerFactory, IJobFactory jobFactory)
        {
            _schedulerFactory = schedulerFactory;
            _jobFactory = jobFactory;
        }

        protected override async Task ExecuteAsync(CancellationToken cancellationToken)
        {
            Scheduler = await _schedulerFactory.GetScheduler(cancellationToken);
            Scheduler.JobFactory = _jobFactory;

            await ScheduledJobs();

            await Scheduler.Start(cancellationToken);
        }

        private async Task ScheduledJobs()
        {
            var jobDetail = JobBuilder.Create(typeof(SampleJob))
                            .WithIdentity(nameof(SampleJob))
                            .Build();

            var trigger = TriggerBuilder.Create()
                    .WithIdentity(nameof(SampleJob))
                    .StartNow()
                    .WithSimpleSchedule(x => x.WithIntervalInMinutes(5).RepeatForever())
                    .Build()
                    ;

            await Scheduler.ScheduleJob(jobDetail, trigger);
        }
    }
}
