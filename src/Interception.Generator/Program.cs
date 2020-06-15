﻿using CommandLine;
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

                    var strict = new List<StrictInterceptionInfo>();
                    var methodFinders = new List<MethodFinderInfo>();
                    var attributed = new List<AttributedInterceptor>();
                    //var skip = File.ReadAllLines(opts.Skip);

                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        strict.AddRange(ProcessStrictInterceptors(assembly));
                        methodFinders.AddRange(ProcessMethodFinders(assembly));
                        attributed.AddRange(ProcessAttributedInterceptors(assembly));
                    }

                    var result = new {
                        attributed,
                        assemblies = opts.Assemblies.Select(a => Path.Combine(opts.Path, new FileInfo(a).Name)).ToList(),
                        composedInterceptor = new
                        {
                            TypeName = typeof(ComposedInterceptor).FullName,
                            AssemblyName = typeof(ComposedInterceptor).Assembly.GetName().Name
                        },
                        interceptorInterface = new
                        {
                            TypeName = typeof(IInterceptor).FullName,
                            AssemblyName = typeof(IInterceptor).Assembly.GetName().Name
                        },
                        methodFinderInterface = new
                        {
                            TypeName = typeof(IMethodFinder).FullName,
                            AssemblyName = typeof(IMethodFinder).Assembly.GetName().Name
                        },
                        methodFinders,
                        skipAssemblies = new[] {
                            //"Anonymously Hosted DynamicMethods Assembly",
                            "Interception.AspNetCore.Tracing",
                            "Interception.AspNetCore.WebHost",
                            "Interception.Cache",
                            "Interception.Core",
                            "Interception.DeadlockDetection",
                            "Interception.DependencyInjection",
                            "Interception.MassTransit",
                            "Interception.Observers",
                            "Interception.OpenTracing.Prometheus",
                            "Interception.Quartz",
                            "Interception.Tracing",
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
                        }.OrderBy(s => s),
                        strict,
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
                        Interceptor = new TypeInfo
                        {
                            AssemblyName = assembly.GetName().Name,
                            TypeName = info.type.FullName
                        }
                    };
                })
                .ToList();

            return result;
        }

        private static List<StrictInterceptionInfo> ProcessStrictInterceptors(Assembly assembly)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Any())
                .SelectMany(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Select(attribute => new { type, attribute = (StrictInterceptAttribute)attribute }))
                .Select(info =>
                {
                    return new StrictInterceptionInfo
                    {
                        IgnoreCallerAssemblies = info.attribute.IgnoreCallerAssemblies?.OrderBy(a => a).Distinct().ToArray(),
                        Target = new TargetMethod
                        {
                            AssemblyName = info.attribute.TargetAssemblyName,
                            MethodName = info.attribute.TargetMethodName,
                            TypeName = info.attribute.TargetTypeName,
                            MethodParametersCount = info.attribute.TargetMethodParametersCount,
                        },
                        Interceptor = new TypeInfo
                        {
                            AssemblyName = info.type.Assembly.GetName().Name,
                            TypeName = info.type.FullName,
                        }
                    };
                })
                .ToList();

            return interceptors;
        }

        private static List<MethodFinderInfo> ProcessMethodFinders(Assembly assembly)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(c => typeof(MethodFinderAttribute).IsAssignableFrom(c.GetType())).Any())
                .SelectMany(type => type.GetCustomAttributes().Where(c => typeof(MethodFinderAttribute).IsAssignableFrom(c.GetType())).Select(attribute => new { type, attribute = (MethodFinderAttribute)attribute }))
                .Select(info =>
                {
                    return new MethodFinderInfo
                    {
                        Target = new TargetMethod
                        {
                            AssemblyName = info.attribute.TargetAssemblyName,
                            MethodName = info.attribute.TargetMethodName,
                            TypeName = info.attribute.TargetTypeName,
                            MethodParametersCount = info.attribute.TargetMethodParametersCount,
                        },
                        MethodFinder = new TypeInfo
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
