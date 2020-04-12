using Interception.Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace Interception
{
    public static class WebHostExtensions
    {
        //[Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Build", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostBuilder", TargetMethodParametersCount = 0)]
        public static object Build(object host, int mdToken, long moduleVersionPtr)
        {
            Metrics.Configure();
            return MethodExecutor.ExecuteMethod(host, new object[] {  }, mdToken, moduleVersionPtr);
        }
    }
}
