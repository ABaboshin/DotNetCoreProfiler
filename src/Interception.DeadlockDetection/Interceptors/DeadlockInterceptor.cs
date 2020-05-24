using Interception.Core;

namespace Interception.DeadlockDetection.Interceptors
{
    public abstract class DeadlockInterceptor : BaseInterceptor
    {
        public override bool SkipExecution()
        {
            return true;
        }
    }
}
