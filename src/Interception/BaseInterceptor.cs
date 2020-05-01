using Interception.Base;
using Interception.Tracing.Extensions;
using OpenTracing;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading.Tasks;

namespace Interception
{
    public abstract class BaseInterceptor
    {
        protected IMethodFinder _methodFinder = new MethodFinder();

        protected IMethodExecutor _methodExecutor = new MethodExecutor();

        protected List<object> _parameters = new List<object>();

        protected object _this;

        protected int _mdToken;

        protected long _moduleVersionPtr;

        public object SetThis(object _this)
        {
            Console.WriteLine($"SetThis {_this}");
            this._this = _this;
            return this;
        }

        public object AddParameter(object value)
        {
            Console.WriteLine($"AddParameter {value}");
            _parameters.Add(value);
            return this;
        }

        public object SetMdToken(int mdToken)
        {
            Console.WriteLine($"SetMdToken {mdToken}");
            _mdToken = mdToken;
            return this;
        }

        public object SetModuleVersionPtr(long moduleVersionPtr)
        {
            Console.WriteLine($"SetModuleVersionPtr {moduleVersionPtr}");
            _moduleVersionPtr = moduleVersionPtr;
            return this;
        }

        public abstract object Execute();

        protected virtual IScope CreateScope()
        {
            throw new NotImplementedException();
        }

        protected virtual MethodBase FindMethod()
        { 
            return _methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
        }

        protected object ExecuteInternal(bool metricsEnabled)
        {
            var method = FindMethod();
            var isAsync = _methodExecutor.IsReturnTypeTask(method);

            if (!metricsEnabled)
            {
                return _methodExecutor.ExecuteSync(method, _this, _parameters.ToArray());
            }

            if (!isAsync)
            {
                return ExecuteSyncInternal();
            }
            else
            {
                return ExecuteAsyncInternal();
            }
        }

        protected object ExecuteSyncInternal()
        {
            var method = FindMethod();

            using (var scope = CreateScope())
            {
                try
                {
                    return _methodExecutor.ExecuteSync(method, _this, _parameters.ToArray());
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }

        protected async Task ExecuteAsyncInternal()
        {
            var method = FindMethod();

            using (var scope = CreateScope())
            {
                try
                {
                    await _methodExecutor.ExecuteAsync(method, _this, _parameters.ToArray());
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }
    }
}
