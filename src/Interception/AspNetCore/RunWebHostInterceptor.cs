using Interception.Attributes;
using Interception.Base;
using Microsoft.AspNetCore.Hosting;
using System;
using System.Threading;

namespace Interception.AspNetCore
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "RunAsync", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostExtensions", TargetMethodParametersCount = 3)]
    public class RunWebHostInterceptor : BaseInterceptor
    {
        protected override void ExecuteAfter(object result, Exception exception)
        {
        }

        protected override void ExecuteBefore()
        {
            Console.WriteLine($"RunWebHostInterceptor this: {_this} {_parameters[0]} {_parameters[1]} {_parameters[2]}");

            DependencyInjection.ServiceProvider = ((IWebHost)_parameters[0]).Services;

            var cts = (CancellationTokenSource)_parameters[1];
            if (cts != null)
            {
                _parameters[1] = cts.Token;
            }
        }
    }
}
