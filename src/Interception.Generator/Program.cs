using CommandLine;
using Interception.Attributes;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Interception.Generator
{
    class Program
    {
        static void Main(string[] args)
        {
            Parser.Default.ParseArguments<Options>(args)
                .WithParsed(opts => {

                    var strict = new List<StrictInterception>();
                    var attributed = new List<AttributedInterceptor>();
                    var initializers = new List<Initializer>();

                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        strict.AddRange(ProcessStrictInterceptors(assembly));
                        attributed.AddRange(ProcessAttributedInterceptors(assembly));

                        var initializer = ProcessInizializers(assembly);
                        if (initializer is null)
                        {
                            initializer = new Initializer { AssemblyName = assembly.GetName().Name };
                        }

                        initializer.AssemblyPath = Path.Combine(opts.Path, new FileInfo(assemblyPath).Name);
                        initializers.Add(initializer);
                    }

                    var result = new {
                        strict,
                        attributed,
                        initializers
                    };

                    File.WriteAllText(opts.Output, JsonConvert.SerializeObject(result, Formatting.Indented));
                });
        }

        private static Initializer ProcessInizializers(Assembly assembly)
        {
            var result = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<InitializeAttribute>().Any())
                .Select(type => new Initializer
                {
                    AssemblyName = assembly.GetName().Name,
                    InitializerType = type.FullName
                })
                .FirstOrDefault();
            return result;
        }

        private static IEnumerable<AttributedInterceptor> ProcessAttributedInterceptors(Assembly assembly)
        {
            var result = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<MethodInterceptorImplementationAttribute>().Any())
                .SelectMany(type => type.GetCustomAttributes<MethodInterceptorImplementationAttribute>().Select(attribute => new { type, attribute }))
                .Select(info => {
                    return new AttributedInterceptor
                    {
                        AttributeType = info.attribute.MethodInterceptorAttribute.FullName,
                        Interceptor = new Interceptor
                        {
                            AssemblyName = assembly.GetName().Name,
                            TypeName = info.type.FullName
                        },
                        ParameterLevel = info.attribute.MethodInterceptorAttribute == typeof(ParameterValidationAttribute)
                    };
                })
                .ToList();

            return result;
        }

        private static IEnumerable<StrictInterception> ProcessStrictInterceptors(Assembly assembly)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<StrictInterceptAttribute>().Any())
                .SelectMany(type => type.GetCustomAttributes<StrictInterceptAttribute>().Select(attribute => new { type, attribute }))
                .Select(info =>
                {
                    return new StrictInterception
                    {
                        CallerAssembly = info.attribute.CallerAssembly,
                        Target = new TargetMethod
                        {
                            AssemblyName = info.attribute.TargetAssemblyName,
                            MethodName = info.attribute.TargetMethodName,
                            TypeName = info.attribute.TargetTypeName,
                            MethodParametersCount = info.attribute.TargetMethodParametersCount,
                        },
                        Interceptor = new Interceptor
                        {
                            AssemblyName = info.type.Assembly.GetName().Name,
                            TypeName = info.type.FullName,
                        }
                    };
                })
                .ToList();

            return interceptors;
        }
    }
}
