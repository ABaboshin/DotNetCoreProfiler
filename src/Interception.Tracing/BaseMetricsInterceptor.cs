using Interception.Core;
using Interception.Tracing.Extensions;
using OpenTracing;
using System;

namespace Interception.Tracing
{
    /// <summary>
    /// base tracing interceptor
    /// </summary>
    public abstract class BaseMetricsInterceptor : BaseAttributedInterceptor
    {
        protected IScope _scope;

        public bool Enabled { get; set; }

        protected BaseMetricsInterceptor(bool enabled)
        {
            Enabled = enabled;
        }

        public override void ExecuteBefore()
        {
            if (Enabled)
            {
                CreateScope();
            }
        }

        public override void ExecuteAfter(object result, Exception exception)
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
