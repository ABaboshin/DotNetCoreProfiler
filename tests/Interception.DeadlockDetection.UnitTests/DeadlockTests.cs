using NUnit.Framework;
using System;
using System.Threading;

namespace Interception.DeadlockDetection.UnitTests
{
    public class DeadlockTests
    {
        public static object o1 = "o1";
        public static object o2 = "o2";

        public static void T1()
        {
            using (new MonitorWrapper(o1))
            {
                Thread.Sleep(1000);
                using (new MonitorWrapper(o2))
                {

                }
            }
        }

        public static void T2()
        {
            using (new MonitorWrapper(o2))
            {
                Thread.Sleep(1000);
                using (new MonitorWrapper(o1))
                {

                }
            }
        }

        [Test]
        public void MonitorTest()
        {
            var t1 = new Thread(() => {
                try
                {
                    T1();
                    Assert.Fail();
                }
                catch (Exception)
                {
                    Assert.AreEqual(1, 1);
                }
            });
            var t2 = new Thread(() => {
                try
                {
                    T2();
                    Assert.Fail();
                }
                catch (Exception)
                {
                    Assert.AreEqual(1, 1);
                }
            });

            t1.Start();
            t2.Start();

            t1.Join();
            t2.Join();
        }
    }
}
