using Interception.Tracing.Extensions;
using OpenTracing;
using System;

namespace Interception.Base
{
    public abstract class BaseMetricsInterceptor : BaseInterceptor
    {
        protected IScope _scope;

        public bool Enabled { get; set; }

        protected BaseMetricsInterceptor(bool enabled)
        {
            Enabled = enabled;
        }

        protected override void ExecuteBefore()
        {
            if (Enabled)
            {
                CreateScope();
            }
        }

        protected override void ExecuteAfter(object result, Exception exception)
        {
            if (_scope != null)
            {
                if (exception != null)
                {
                    _scope.Span.SetException(exception);
                }

                _scope.Span.SetTag("result", result?.ToString());

                _scope.Dispose();
                _scope = null;
            }
        }

        protected abstract void CreateScope();
    }
}
