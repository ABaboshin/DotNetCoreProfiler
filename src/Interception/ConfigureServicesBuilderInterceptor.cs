using Interception.Common;
using Interception.Observers;
using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System.Diagnostics;

namespace Interception
{
    public static class ConfigureServicesBuilderInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
        public static object Invoke(object builder, object instance, object services, int mdToken, long moduleVersionPtr)
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var httpConfiguration = configuration.GetSection(HttpConfiguration.SectionKey).Get<HttpConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(httpConfiguration));

            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);
        }
    }
}
