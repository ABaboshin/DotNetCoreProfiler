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

        public override void ExecuteAfter()
        {
            if (_scope != null)
            {
                if (Exception != null)
                {
                    _scope.Span.SetException(Exception);
                }

                _scope.Span.SetTag("result", Result?.ToString());

                _scope.Dispose();
                _scope = null;
            }
        }

        protected abstract void CreateScope();
    }
}
