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
    public class QuartzJobExecuteInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            var options = DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>();

            Console.WriteLine($"options {options.Value}");

            return ExecuteInternal(options.Value.Enabled);
        }

        protected override IScope CreateScope()
        {
            var consumerName = _this.GetType().FullName;

            var spanBuilder = GlobalTracer.Instance
                    .BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<QuartzConfiguration>>().Value.Name)
                    .WithTag(Tags.SpanKind, Tags.SpanKindServer)
                    .WithTag("task", consumerName);

            return spanBuilder.StartActive(true);
        }
    }
}
