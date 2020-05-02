using Interception.Attributes;
using Interception.Base;
using OpenTracing;
using OpenTracing.Util;
using System;
using System.Linq;

namespace Interception
{
    [MonitoringIntercept]
    public class MonitoringInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            return ExecuteInternal(true);
        }

        protected override void EnrichAfterExecution(object result, IScope scope)
        {
            scope.Span.SetTag("result", result.ToString());

            base.EnrichAfterExecution(result, scope);
        }

        protected override IScope CreateScope()
        {
            var mb = _methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
            var attr = (MonitorAttribute)mb.GetCustomAttributes(typeof(MonitorAttribute), false).FirstOrDefault();

            var spanBuilder = GlobalTracer.Instance.BuildSpan(attr.Name).AsChildOf(GlobalTracer.Instance.ActiveSpan);

            if (attr.Parameters != null && attr.Parameters.Any())
            {
                var methodParameters = mb.GetParameters().ToList();

                foreach (var p in attr.Parameters)
                {
                    var index = methodParameters.FindIndex(mp => mp.Name == p);
                    if (index != -1)
                    {
                        spanBuilder = spanBuilder
                            .WithTag($"parameter.{p}", _parameters[index]?.ToString());
                    }
                }
            }

            return spanBuilder.StartActive();
        }
    }
}
