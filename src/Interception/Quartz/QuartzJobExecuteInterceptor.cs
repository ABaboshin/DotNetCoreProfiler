using Interception.AspNetCore;
using Interception.Attributes;
using Microsoft.Extensions.Options;
using OpenTracing.Tag;
using OpenTracing.Util;
using Microsoft.Extensions.DependencyInjection;
using Interception.Tracing;

namespace Interception.Quartz
{
    /// <summary>
    /// intercept quartz job running
    /// and inject tracing
    /// </summary>
    [StrictIntercept(CallerAssembly = "", TargetAssemblyName = "Quartz", TargetMethodName = "Execute", TargetTypeName = "Quartz.IJob", TargetMethodParametersCount = 1)]
    public class QuartzJobExecuteInterceptor : BaseMetricsInterceptor
    {
        public QuartzJobExecuteInterceptor() : base(DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>().Value.Enabled)
        {
        }

        protected override void CreateScope()
        {
            var consumerName = This.GetType().FullName;

            var spanBuilder = GlobalTracer.Instance
                    .BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>().Value.Name)
                    .WithTag(Tags.SpanKind, Tags.SpanKindServer)
                    .WithTag("task", consumerName);

            _scope = spanBuilder.StartActive(true);
        }
    }
}
