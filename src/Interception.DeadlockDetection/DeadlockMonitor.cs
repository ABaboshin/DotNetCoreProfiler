using Interception.DeadlockDetection.DataStructures;
using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;

namespace Interception.DeadlockDetection
{
    internal class DeadlockMonitor
    {
        private readonly ConditionalWeakTable<Thread, ConcurrentBag<DeadlockException>> _pendingDeadlockExceptions = new ConditionalWeakTable<Thread, ConcurrentBag<DeadlockException>>();
        private readonly Graph _graph = new Graph();
        private readonly object _deadlockDetectionLock = new object();

        #region current
        private static volatile DeadlockMonitor _current;
        public static DeadlockMonitor Current
        {
            get
            {
                if (_current == null)
                {
                    var deadlockMonitor = new DeadlockMonitor();
                    Interlocked.CompareExchange(ref _current, deadlockMonitor, null);
                }
                return _current;
            }
        }
        #endregion

        public void Acquire(Action enterWaiting, Func<int, bool> tryAcquire, Action convertWaitingToAcquired, Action exitWaiting)
        {
            var inWaiting = false;
            try
            {
                // try to lock the resource without timeout
                if (tryAcquire(0))
                {
                    // set as acquited
                    convertWaitingToAcquired();
                    return;
                }

                // set as waiting
                enterWaiting();
                inWaiting = true;

                // check for deadlock exceptions
                // they can come from another threads
                var pendingException = GetPendingException();
                if (pendingException != null)
                {
                    throw pendingException;
                }

                // try to acquire with a timeout
                bool acquired = tryAcquire(200);

                // check for exceptions
                pendingException = GetPendingException();

                if (pendingException == null)
                {
                    if (!acquired)
                    {
                        // if acquiring failed and no exception was collected
                        // do it now
                        DetectDeadlocks(Thread.CurrentThread);
                    }
                    else
                    {
                        // set as acquired
                        convertWaitingToAcquired();
                        inWaiting = false;
                    }
                }
                else
                {
                    throw pendingException;
                }
            }
            finally
            {
                if (inWaiting)
                {
                    // remove waiting edge
                    exitWaiting();
                }
            }
        }

        public void EnterWaiting(object monitorObject, LockType lockType)
        {
            AddEdge(Thread.CurrentThread, LockType.Thread, monitorObject, lockType);
        }

        public void ConvertWaitingToAcquired(object monitorObject, LockType lockType)
        {
            RemoveEdge(Thread.CurrentThread, LockType.Thread, monitorObject, lockType);
            AddEdge(monitorObject, lockType, Thread.CurrentThread, LockType.Thread);
        }

        public void ExitWaiting(object monitorObject, LockType lockType)
        {
            RemoveEdge(Thread.CurrentThread, LockType.Thread, monitorObject, lockType);
        }

        public void EnterAcquired(object monitorObject, LockType lockType)
        {
            AddEdge(monitorObject, lockType, Thread.CurrentThread, LockType.Thread);
        }

        public void ExitAcquired(object monitorObject, LockType lockType)
        {
            RemoveEdge(monitorObject, lockType, Thread.CurrentThread, LockType.Thread);
        }

        private void AddEdge(object from, LockType fromLockType, object to, LockType toLockType)
        {
            _graph.AddEdge(new Node(from, fromLockType), new Node(to, toLockType));
        }

        private void RemoveEdge(object from, LockType fromLockType, object to, LockType toLockType)
        {
            _graph.RemoveEdge(new Node(from, fromLockType), new Node(to, toLockType));
        }

        internal Exception GetPendingException()
        {
            if (!_pendingDeadlockExceptions.TryGetValue(Thread.CurrentThread, out var concurrentBag))
                return null;

            return concurrentBag.Count() > 1 ? (Exception)new AggregateException(concurrentBag) : concurrentBag.FirstOrDefault();
        }

        internal void DetectDeadlocks(Thread startThread)
        {
            lock (_deadlockDetectionLock)
            {
                _graph.FindCycles(new Node(startThread, LockType.Thread), out var cycles);

                if (!cycles.Any())
                {
                    return;
                }

                var info = new DeadlockInfo
                {
                    Threads = cycles
                    .Where(e => e.Prev.LockType == LockType.Thread)
                    .Select(e => e.Prev.MonitorObject)
                    .OfType<Thread>()
                    .Union(
                    cycles
                    .Where(e => e.Next.LockType == LockType.Thread)
                    .Select(e => e.Next.MonitorObject)
                    .OfType<Thread>()
                    ).ToList(),
                    Message = string.Join("; ", cycles.Select(c => c.ToString()))
                };

                foreach (var thread in info.Threads.Where(t => t != Thread.CurrentThread))
                {
                    _pendingDeadlockExceptions
                        .GetValue(thread, t => new ConcurrentBag<DeadlockException>())
                        .Add(new DeadlockException(info));
                }

                throw new DeadlockException(info);
            }
        }
    }
}
