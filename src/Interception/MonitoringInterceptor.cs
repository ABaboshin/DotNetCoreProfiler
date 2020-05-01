using Interception.Attributes;
using OpenTracing;
using OpenTracing.Util;

namespace Interception
{
    [MonitoringIntercept]
    public class MonitoringInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            return ExecuteInternal(true);
        }

        protected override IScope CreateScope()
        {
            return GlobalTracer.Instance.BuildSpan("Monitoring").AsChildOf(GlobalTracer.Instance.ActiveSpan).StartActive();
        }
    }
}
