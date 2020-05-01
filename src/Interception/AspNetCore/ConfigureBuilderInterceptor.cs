using Interception.Attributes;
using Interception.Base;
using Interception.Common.Extensions;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Reflection;

namespace Interception.AspNetCore
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Invoke", TargetTypeName = "Microsoft.AspNetCore.Hosting.Internal.ConfigureBuilder", TargetMethodParametersCount = 2)]
    public class ConfigureBuilderInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var aspNetCoreConfiguration = configuration.GetSection(AspNetCoreConfiguration.SectionKey).Get<AspNetCoreConfiguration>();

            if (!aspNetCoreConfiguration.Enabled)
            {
                return ExecuteInternal(false);
            }

            var instance = _parameters[0];
            var builder = (IApplicationBuilder)_parameters[1];
            using (var scope = builder.ApplicationServices.CreateScope())
            {
                var serviceProvider = scope.ServiceProvider;
                var methodInfo = _this.GetPropertyValue<MethodInfo>("MethodInfo");
                var parameters1 = methodInfo.GetParameters();
                var parameters2 = new object[parameters1.Length];
                for (int index = 0; index < parameters1.Length; ++index)
                {
                    var parameterInfo = parameters1[index];
                    if (parameterInfo.ParameterType == typeof(IApplicationBuilder))
                    {
                        parameters2[index] = (object)builder;
                    }
                    else
                    {
                        try
                        {
                            parameters2[index] = serviceProvider.GetRequiredService(parameterInfo.ParameterType);
                        }
                        catch (Exception ex)
                        {
                            throw new Exception(string.Format("Could not resolve a service of type '{0}' for the parameter '{1}' of method '{2}' on type '{3}'.", (object)parameterInfo.ParameterType.FullName, (object)parameterInfo.Name, (object)methodInfo.Name, (object)methodInfo.DeclaringType.FullName), ex);
                        }
                    }
                }

                Console.WriteLine("Use tracing middleware");
                builder.UseTracingMiddleware();
                methodInfo.Invoke(instance, parameters2);
            }

            return null;
        }
    }
}
