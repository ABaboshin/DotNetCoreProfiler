using CommandLine;
using Interception.Attributes;
using Interception.Core;
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

                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        strict.AddRange(ProcessStrictInterceptors(assembly));
                        attributed.AddRange(ProcessAttributedInterceptors(assembly));
                    }

                    var result = new {
                        strict,
                        attributed,
                        assemblies = opts.Assemblies.Select(a => Path.Combine(opts.Path, new FileInfo(a).Name)).ToList(),
                        baseClass = new { 
                            TypeName = typeof(IInterceptor).FullName,
                            AssemblyName = typeof(IInterceptor).Assembly.GetName().Name
                        },
                        composed = new
                        {
                            TypeName = typeof(ComposedInterceptor).FullName,
                            AssemblyName = typeof(ComposedInterceptor).Assembly.GetName().Name
                        },
                        skipAssemblies = new[] {
                            "Anonymously Hosted DynamicMethods Assembly",
                            "Interception",
                            "Interception.Common",
                            "Interception.Observers",
                            "ISymWrapper",
                            "Microsoft.AspNetCore.Mvc.RazorPages",
                            "Microsoft.AspNetCore.Razor.Language",
                            "Microsoft.CSharp",
                            "Microsoft.Extensions.ObjectPool",
                            "Microsoft.Extensions.Options",
                            "mscorlib",
                            "netstandard",
                            "Newtonsoft.Json",
                            "StatsdClient",
                            "System.Collections",
                            "System.ComponentModel",
                            "System.Configuration",
                            "System.Console",
                            "System.Core",
                            "System.Diagnostics.DiagnosticSource",
                            "System.IO.FileSystem",
                            "System.Private.CoreLib",
                            "System.Runtime",
                            "System.Runtime.Extensions",
                            "System.Runtime.InteropServices",
                            "System.Runtime.InteropServices.RuntimeInformation",
                            "System.Threading.Tasks",
                            "System.Xml.Linq"
                        }.OrderBy(s => s)
                    };

                    File.WriteAllText(opts.Output, JsonConvert.SerializeObject(result, Formatting.Indented));
                });
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
                        }
                    };
                })
                .ToList();

            return result;
        }

        private static List<StrictInterception> ProcessStrictInterceptors(Assembly assembly)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Any())
                .SelectMany(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Select(attribute => new { type, attribute = (StrictInterceptAttribute)attribute }))
                .Select(info =>
                {
                    return new StrictInterception
                    {
                        IgnoreCallerAssemblies = info.attribute.IgnoreCallerAssemblies?.OrderBy(a => a).Distinct().ToArray(),
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
