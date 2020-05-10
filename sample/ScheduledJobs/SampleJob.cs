using Interception.Attributes;
using MassTransit;
using Quartz;
using SampleApp.MessageBus;
using System;
using System.Threading.Tasks;

namespace SampleApp.ScheduledJobs
{
    public class SampleJob : IJob
    {
        //private readonly IBusControl _busControl;

        //public SampleJob(IBusControl busControl)
        //{
        //    _busControl = busControl;
        //}

        public async Task Execute(IJobExecutionContext context)
        {
            //await _busControl.Publish(new MyMessage { Id = 3 });

            FibonacciCall();

            FibonacciCall();
        }

        [Monitor(Name = "Fibonacci call")]
        void FibonacciCall()
        {
            Fibonacci(100);
        }

        [Cache(DurationSeconds = 60, Parameters = new[] { "n" })]
        int Fibonacci(int n)
        {
            if (n < 0)
            {
                throw new ArgumentOutOfRangeException();
            }

            if (n == 0)
            {
                return 0;
            }

            if (n == 1)
            {
                return 1;
            }

            return Fibonacci(n - 1) + Fibonacci(n - 2);
        }
    }
}
