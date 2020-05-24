using Interception.DeadlockDetection.DataStructures;
using System;
using System.Threading;

namespace Interception.DeadlockDetection.Enhancements
{
    internal static class MonitorEnhancements
    {
        internal static void Enter(object obj, ref bool lockTaken)
        {
            var copy = lockTaken;
            DeadlockMonitor.Current.Acquire(
                () => DeadlockMonitor.Current.EnterWaiting(obj, LockType.Monitor),
                timeout =>
                {
                    Console.WriteLine($"try lock {obj} with timeout {timeout}");
                    Monitor.TryEnter(obj, timeout, ref copy);
                    Console.WriteLine($"result {copy} lock {obj} with timeout {timeout}");
                    return copy;
                },
                () => DeadlockMonitor.Current.ConvertWaitingToAcquired(obj, LockType.Monitor),
                () => DeadlockMonitor.Current.ExitWaiting(obj, LockType.Monitor)
                );
        }

        internal static void TryEnter(object obj, TimeSpan timeSpan, ref bool lockTaken)
        {
            Monitor.TryEnter(obj, timeSpan, ref lockTaken);
            if (!lockTaken)
                return;
            DeadlockMonitor.Current.EnterAcquired(obj, LockType.Monitor);
        }

        internal static void Exit(object obj)
        {
            Monitor.Exit(obj);
            DeadlockMonitor.Current.ExitAcquired(obj, LockType.Monitor);
        }
    }
}
