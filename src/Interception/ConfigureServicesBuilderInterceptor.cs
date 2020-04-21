using Interception.Common;
using Interception.Observers;
using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System;
using System.Diagnostics;

namespace Interception
{
    public static class ConfigureServicesBuilderInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
        public static object Invoke(object builder, object instance, object services, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"Configure additional services {builder.GetType().Name} {instance.GetType().Name} {services.GetType().Name}");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var aspNetCoreConfiguration = configuration.GetSection(AspNetCoreConfiguration.SectionKey).Get<AspNetCoreConfiguration>();
            var httpHandlerConfiguration = configuration.GetSection(HttpHandlerConfiguration.SectionKey).Get<HttpHandlerConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(aspNetCoreConfiguration, httpHandlerConfiguration));

            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);
        }
    }
}
