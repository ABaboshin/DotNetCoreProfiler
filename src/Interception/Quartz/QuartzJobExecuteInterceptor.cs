using Interception.AspNetCore;
using Interception.Attributes;
using Interception.Base;
using Microsoft.Extensions.Options;
using OpenTracing;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using Microsoft.Extensions.DependencyInjection;

namespace Interception.Quartz
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "Quartz", TargetMethodName = "Execute", TargetTypeName = "Quartz.IJob", TargetMethodParametersCount = 1)]
    public class QuartzJobExecuteInterceptor : BaseMetricsInterceptor
    {
        public QuartzJobExecuteInterceptor() : base(DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>().Value.Enabled)
        {
        }

        protected override void CreateScope()
        {
            var consumerName = _this.GetType().FullName;

            var spanBuilder = GlobalTracer.Instance
                    .BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>().Value.Name)
                    .WithTag(Tags.SpanKind, Tags.SpanKindServer)
                    .WithTag("task", consumerName);

            _scope = spanBuilder.StartActive(true);
        }
    }
}
