using Interception.Attributes;
using Interception.Base;
using OpenTracing.Util;
using System.Linq;

namespace Interception
{
    [MonitoringIntercept]
    public class MonitoringInterceptor : BaseMetricsInterceptor
    {
        public MonitoringInterceptor() : base(true)
        {
        }

        protected override void CreateScope()
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

            _scope = spanBuilder.StartActive();
        }
    }
}
