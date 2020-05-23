using Interception.Attributes;
using Interception.Core;
using Microsoft.AspNetCore.Hosting;
using System.Threading;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept WebHost.Run
    /// and get the DI
    /// </summary>
    [StrictIntercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "RunAsync", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostExtensions", TargetMethodParametersCount = 3)]
    public class RunWebHostInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            DependencyInjection.ServiceProvider = ((IWebHost)GetParameter(0)).Services;

            var cts = (CancellationTokenSource)GetParameter(1);
            if (cts != null)
            {
                ModifyParameter(1, cts.Token);
            }
        }
    }
}
