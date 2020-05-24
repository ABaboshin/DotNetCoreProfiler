using System;

namespace Interception.DeadlockDetection
{
    public class DeadlockException : Exception
    {
        public DeadlockInfo DeadlockInfo { get; set; }

        public DeadlockException(DeadlockInfo deadlockInfo) : base(deadlockInfo.Message)
        {
            DeadlockInfo = deadlockInfo;
        }
    }
}
