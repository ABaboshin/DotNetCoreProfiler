using Interception.Attributes;
using Interception.Extensions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Interception
{
    public class Loader
    {
        public Loader(string interceptionDlls)
        {
            Console.WriteLine($"Interception.Loader {interceptionDlls}");

            var assemblies = new List<Assembly>();
            var attributedInterceptors = new List<Type>();

            foreach (var dll in interceptionDlls.Split(new char[] { ',' }))
            {
                var assembly = Assembly.LoadFrom(dll);

                PorcessStrictInterceptors(assembly);
                ProcessInitializer(assembly);

                assemblies.Add(assembly);

                attributedInterceptors.AddRange(FindAttributedInterceptors(assembly));
            }

            foreach (var attributedInterceptor in attributedInterceptors)
            {
                ProcessAttributedInterceptor(attributedInterceptor);
            }
        }

        private static void ProcessAttributedInterceptor(Type attributedInterceptor)
        {
            var attribute = attributedInterceptor.GetCustomAttributes().Where(a => a.GetType().GetInterfaces().Where(i => i.Name == nameof(IInterceptorAttribute)).Any()).First();
            var userAttribute = attribute.GetPropertyValue<Type>(nameof(IInterceptorAttribute.UserAttributeType));

            var attributedMethods = AppDomain.CurrentDomain.GetAssemblies()
                .SelectMany(a => a.GetTypes())
                .SelectMany(t => t.GetRuntimeMethods())
                .Where(m => m.GetCustomAttributes().Where(a => a.GetType().FullName == userAttribute.FullName).Any())
                .ToList();

            foreach (var attributedMethod in attributedMethods)
            {
                NativeMethods.AddInterceptor(new ImportInterception
                {
                    CallerAssembly = "",
                    InterceptorAssemblyName = attributedInterceptor.Assembly.GetName().Name,
                    InterceptorTypeName = attributedInterceptor.FullName,
                    TargetAssemblyName = attributedMethod.DeclaringType.Assembly.GetName().Name,
                    TargetTypeName = attributedMethod.DeclaringType.FullName,
                    TargetMethodName = attributedMethod.Name,
                    TargetMethodParametersCount = attributedMethod.GetParameters().Length
                });
            }
        }

        private IEnumerable<Type> FindAttributedInterceptors(Assembly assembly)
        {
            return assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(a => a.GetType().GetInterfaces().Where(i => i.Name == nameof(IInterceptorAttribute)).Any()).Any())
                .ToList();
        }

        private void ProcessInitializer(Assembly assembly)
        {
            var initializers = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(InitializeAttribute).FullName).Any())
                .ToList();

            foreach (var initializer in initializers)
            {
                Activator.CreateInstance(initializer);
            }
        }

        private void PorcessStrictInterceptors(Assembly assembly)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(StrictInterceptAttribute).FullName).Any())
                .SelectMany(type => type.GetCustomAttributes().Select(attribute => new { type, attribute }))
                .Where(info => info.attribute.GetType().FullName == typeof(StrictInterceptAttribute).FullName)
                .Select(info =>
                {
                    return new ImportInterception
                    {
                        CallerAssembly = info.attribute.GetPropertyValue<string>(nameof(StrictInterceptAttribute.CallerAssembly)),
                        TargetAssemblyName = info.attribute.GetPropertyValue<string>(nameof(StrictInterceptAttribute.TargetAssemblyName)),
                        TargetMethodName = info.attribute.GetPropertyValue<string>(nameof(StrictInterceptAttribute.TargetMethodName)),
                        TargetTypeName = info.attribute.GetPropertyValue<string>(nameof(StrictInterceptAttribute.TargetTypeName)),
                        TargetMethodParametersCount = info.attribute.GetPropertyValue<int>(nameof(StrictInterceptAttribute.TargetMethodParametersCount)),
                        InterceptorTypeName = info.type.FullName,
                        InterceptorAssemblyName = info.type.Assembly.GetName().Name,
                    };
                })
                .ToList();

            foreach (var interceptor in interceptors)
            {
                NativeMethods.AddInterceptor(interceptor);
            }
        }
    }
}
