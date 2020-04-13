using Interception.Common;
using Interception.Metrics;
using Interception.Metrics.Configuration;
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

            var statsdConfiguration = configuration.GetSection(StatsdConfiguration.SectionKey).Get<StatsdConfiguration>();
            var httpConfiguration = configuration.GetSection(HttpConfiguration.SectionKey).Get<HttpConfiguration>();
            var serviceConfiguration = configuration.GetSection(ServiceConfiguration.SectionKey).Get<ServiceConfiguration>();

            MetricsSender.Configure(statsdConfiguration, serviceConfiguration);

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(httpConfiguration));

            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);
        }
    }
}
