using Interception.Attributes;
using Interception.Core;
using System;

namespace Interception.Serilog
{
    [StrictIntercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.Extensions.Logging.Abstractions", TargetMethodName = "LogInformation", TargetTypeName = "Microsoft.Extensions.Logging.LoggerExtensions", TargetMethodParametersCount = 3)]
    public class LogInformationInterceptor : BaseInterceptor
    {
        public override int Priority => 100;

        public override void ExecuteBefore()
        {
            Console.WriteLine($"Call LogInformationInterceptor {GetParameter(0)} {GetParameter(1)} {GetParameter(2)}");
            base.ExecuteBefore();
        }
    }
}
