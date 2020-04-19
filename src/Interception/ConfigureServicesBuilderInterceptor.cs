using Interception.Common;
using Interception.Observers;
using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Diagnostics;

namespace Interception
{
    public static class ConfigureServicesBuilderInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureServicesBuilder", TargetMethodParametersCount = 2)]
        public static object Invoke(object builder, object instance, object services, int mdToken, long moduleVersionPtr)
        //[Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "BuildCommonServices", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostBuilder", TargetMethodParametersCount = 1)]
        //public static object Invoke(object builder, AggregateException ex, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine("Configure additional services");
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var httpConfiguration = configuration.GetSection(HttpConfiguration.SectionKey).Get<HttpConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(httpConfiguration));

            var serviceCollection = (IServiceCollection)services;
            serviceCollection.AddOpenTracing();

            return MethodExecutor.ExecuteMethod(builder, new object[] { instance, services }, mdToken, moduleVersionPtr, true);

            //serviceCollection.AddOpenTracing();

            //return serviceCollection;
        }
    }
}
