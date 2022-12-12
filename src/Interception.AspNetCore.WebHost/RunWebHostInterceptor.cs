using System;
using Interception.Attributes;
using Interception.Core;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Hosting;
using System.Threading;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept WebHost.Run
    /// and get the DI
    /// </summary>
    // [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "RunAsync", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostExtensions", TargetMethodParametersCount = 2)]
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Run", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostExtensions", TargetMethodParametersCount = 1)]
    public class RunWebHostInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var webHost = GetParameter(0);
            DependencyInjection.Instance.ServiceProvider = (IServiceProvider)webHost.GetType().GetProperty("Services").GetValue(webHost);

            // var cts = (CancellationTokenSource)GetParameter(1);
            // if (cts != null)
            // {
            //     ModifyParameter(1, cts.Token);
            // }
        }
    }
}
