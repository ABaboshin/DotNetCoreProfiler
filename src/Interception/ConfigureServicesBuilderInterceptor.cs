using Interception.Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace Interception
{
    public static class ConfigureServicesBuilderInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
        public static object Invoke(object builder, object instance, object services, int mdToken, long moduleVersionPtr)
        {
            Metrics.Configure();
            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);
        }
    }
}
