using Interception.Attributes;
using Interception.Extensions;
using System;
using System.Linq;
using System.Reflection;

namespace Interception
{
    public class Loader
    {
        public Loader(string interceptionDlls)
        {
            Console.WriteLine($"Interception.Loader {interceptionDlls}");

            Type monitoringInterceptor = null;
            Type cacheInterceptor = null;

            foreach (var dll in interceptionDlls.Split(new char[] { ',' }))
            {
                Console.WriteLine($"Load {dll}");
                var assembly = Assembly.LoadFrom(dll);

                if (monitoringInterceptor is null)
                {
                    monitoringInterceptor = assembly
                        .GetTypes()
                        .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(MonitoringInterceptAttribute).FullName).Any())
                        .FirstOrDefault();
                }

                if (cacheInterceptor is null)
                {
                    cacheInterceptor = assembly
                        .GetTypes()
                        .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(CacheInterceptorAttribute).FullName).Any())
                        .FirstOrDefault();
                }

                var interceptors = assembly
                    .GetTypes()
                    .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(InterceptAttribute).FullName).Any())
                    .SelectMany(type => type.GetCustomAttributes().Select(attribute => new { type, attribute }))
                    .Where(info => info.attribute.GetType().FullName == typeof(InterceptAttribute).FullName)
                    .Select(info =>
                    {
                        return new ImportInterception
                        {
                            CallerAssembly = info.attribute.GetPropertyValue<string>(nameof(InterceptAttribute.CallerAssembly)),
                            TargetAssemblyName = info.attribute.GetPropertyValue<string>(nameof(InterceptAttribute.TargetAssemblyName)),
                            TargetMethodName = info.attribute.GetPropertyValue<string>(nameof(InterceptAttribute.TargetMethodName)),
                            TargetTypeName = info.attribute.GetPropertyValue<string>(nameof(InterceptAttribute.TargetTypeName)),
                            TargetMethodParametersCount = info.attribute.GetPropertyValue<int>(nameof(InterceptAttribute.TargetMethodParametersCount)),
                            InterceptorTypeName = info.type.FullName,
                            InterceptorAssemblyName = info.type.Assembly.GetName().Name,
                        };
                    })
                    .ToList();

                foreach (var interceptor in interceptors)
                {
                    NativeMethods.AddInterceptor(interceptor);
                }

                var initializers = assembly
                    .GetTypes()
                    .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(InitializeAttribute).FullName).Any())
                    .ToList();

                foreach (var initializer in initializers)
                {
                    Activator.CreateInstance(initializer);
                }
            }

            var monitorMethods = AppDomain.CurrentDomain.GetAssemblies()
                .SelectMany(a => a.GetTypes())
                .SelectMany(t => t.GetRuntimeMethods())
                .Where(m => m.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(MonitorAttribute).FullName).Any())
                .ToList();

            if (monitoringInterceptor != null)
            {
                foreach (var monitor in monitorMethods)
                {
                    NativeMethods.AddInterceptor(new ImportInterception {
                        CallerAssembly = "",
                        InterceptorAssemblyName = monitoringInterceptor.Assembly.GetName().Name,
                        InterceptorTypeName = monitoringInterceptor.FullName,
                        TargetAssemblyName = monitor.DeclaringType.Assembly.GetName().Name,
                        TargetTypeName = monitor.DeclaringType.FullName,
                        TargetMethodName = monitor.Name,
                        TargetMethodParametersCount = monitor.GetParameters().Length
                    });
                }
            }

            Console.WriteLine($"cacheInterceptor {cacheInterceptor}");

            var cacheMethods = AppDomain.CurrentDomain.GetAssemblies()
                .SelectMany(a => a.GetTypes())
                .SelectMany(t => t.GetRuntimeMethods())
                .Where(m => m.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(CacheAttribute).FullName).Any())
                .ToList();

            Console.WriteLine($"cacheMethods {cacheMethods.Count()}");

            if (cacheInterceptor != null)
            {
                foreach (var cache in cacheMethods)
                {
                    NativeMethods.AddInterceptor(new ImportInterception
                    {
                        CallerAssembly = "",
                        InterceptorAssemblyName = cacheInterceptor.Assembly.GetName().Name,
                        InterceptorTypeName = cacheInterceptor.FullName,
                        TargetAssemblyName = cache.DeclaringType.Assembly.GetName().Name,
                        TargetTypeName = cache.DeclaringType.FullName,
                        TargetMethodName = cache.Name,
                        TargetMethodParametersCount = cache.GetParameters().Length
                    });
                }
            }

            Console.WriteLine($"monitor {string.Join(Environment.NewLine, monitorMethods.Select(m => m.Name))}");
        }
    }
}
