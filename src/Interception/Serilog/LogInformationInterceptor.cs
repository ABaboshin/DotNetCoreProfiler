using Interception.Attributes;
using Interception.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Serilog
{
    [StrictIntercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.Extensions.Logging.Abstractions", TargetMethodName = "LogInformation", TargetTypeName = "Microsoft.Extensions.Logging.LoggerExtensions", TargetMethodParametersCount = 3)]
    public class LogInformationInterceptor : BaseInterceptor
    {
        public override void ExecuteBefore()
        {
            Console.WriteLine($"Call LogInformationInterceptor {GetParameter(0)} {GetParameter(1)} {GetParameter(2)}");
            base.ExecuteBefore();
        }
    }
}
