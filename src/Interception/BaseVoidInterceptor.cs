using System;

namespace Interception
{
    public abstract class BaseVoidInterceptor : BaseInterceptor
    {
        public abstract void ExecuteVoid();

        public override object Execute()
        {
            throw new NotImplementedException();
        }
    }
}
