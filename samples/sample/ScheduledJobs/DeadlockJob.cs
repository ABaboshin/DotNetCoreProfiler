using Quartz;
using System.Threading;
using System.Threading.Tasks;

namespace SampleApp.ScheduledJobs
{
    public class DeadlockJob : IJob
    {
        public static object o1 = "o1";
        public static object o2 = "o2";

        public static void T1()
        {
            lock(o1)
            {
                Thread.Sleep(1000);
                lock (o2)
                {

                }
            }
        }

        public static void T2()
        {
            lock (o2)
            {
                Thread.Sleep(1000);
                lock (o1)
                {

                }
            }
        }

        public Task Execute(IJobExecutionContext context)
        {
            var t1 = new Thread(T1);
            var t2 = new Thread(T2);

            t1.Start();
            t2.Start();

            t1.Join();
            t2.Join();

            return Task.CompletedTask;
        }
    }
}
