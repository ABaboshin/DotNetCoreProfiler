using System.Collections.Generic;
using System.Threading;

namespace Interception.DeadlockDetection
{
    public class DeadlockInfo
    {
        public List<Thread> Threads { get; set; }
        public string Message { get; set; }

        public override string ToString()
        {
            return Message;
        }
    }
}
