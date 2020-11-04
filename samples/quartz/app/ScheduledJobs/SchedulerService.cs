using Microsoft.Extensions.Hosting;
using Quartz;
using Quartz.Spi;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace app.ScheduledJobs
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

            await ScheduledJob<SampleJob>();

            await Scheduler.Start(cancellationToken);
        }

        private async Task ScheduledJob<T>() where T : IJob
        {
            var jobDetail = JobBuilder.Create<T>()
                            .WithIdentity(typeof(T).Name)
                            .Build();

            var trigger = TriggerBuilder.Create()
                    .WithIdentity(typeof(T).Name)
                    .StartNow()
                    .WithSimpleSchedule(x => x.WithIntervalInMinutes(1).RepeatForever())
                    .Build()
                    ;

            await Scheduler.ScheduleJob(jobDetail, trigger);
        }
    }
}
