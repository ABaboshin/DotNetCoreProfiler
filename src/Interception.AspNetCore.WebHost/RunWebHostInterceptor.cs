using Interception.Attributes;
using Interception.Core;
using Microsoft.Extensions.Hosting;
using System.Threading;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept WebHost.Run
    /// and get the DI
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting.Abstractions", TargetMethodName = "RunAsync", TargetTypeName = "Microsoft.Extensions.Hosting.HostingAbstractionsHostExtensions", TargetMethodParametersCount = 2)]
    public class RunWebHostInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            DependencyInjection.Instance.ServiceProvider = ((IHost)GetParameter(0)).Services;

            var cts = (CancellationTokenSource)GetParameter(1);
            if (cts != null)
            {
                ModifyParameter(1, cts.Token);
            }
        }
    }
}
