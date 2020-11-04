using Microsoft.Extensions.Logging;
using Quartz;
using System.Threading.Tasks;

namespace app.ScheduledJobs
{
    public class SampleJob : IJob
    {
        private readonly ILogger<SampleJob> _logger;

        public SampleJob(ILoggerFactory loggerFactory)
        {
      System.Console.WriteLine("SampleJob");
            _logger = loggerFactory.CreateLogger<SampleJob>();
        }

        public Task Execute(IJobExecutionContext context)
        {
          System.Console.WriteLine("Execute");
            _logger.LogInformation("test");
            return Task.CompletedTask;
        }
    }
}
