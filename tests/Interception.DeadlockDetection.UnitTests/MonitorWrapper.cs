using Interception.DeadlockDetection.Enhancements;
using System;

namespace Interception.DeadlockDetection.UnitTests
{
    public class MonitorWrapper : IDisposable
    {
        private readonly object _lockObject;

        public MonitorWrapper(object lockObject)
        {
            _lockObject = lockObject;

            var lockTaken = false;
            MonitorEnhancements.Enter(lockObject, ref lockTaken);
        }

        private bool disposed = false;
        public void Dispose()
        {
            Dispose(true);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    MonitorEnhancements.Exit(_lockObject);
                }
            }
        }
    }
}
