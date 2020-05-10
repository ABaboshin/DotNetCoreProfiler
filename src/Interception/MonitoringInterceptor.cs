using Interception.Attributes;
using Interception.Base;
using OpenTracing.Util;
using System;
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
            var method = _methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
            var attribute = (MonitorAttribute)method.GetCustomAttributes(typeof(MonitorAttribute), false).FirstOrDefault();

            var spanBuilder = GlobalTracer.Instance.BuildSpan(attribute.Name).AsChildOf(GlobalTracer.Instance.ActiveSpan);

            Console.WriteLine($"CreateScope {attribute.Name}");

            if (attribute.Parameters != null && attribute.Parameters.Any())
            {
                var methodParameters = method.GetParameters().ToList();

                foreach (var p in attribute.Parameters)
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
