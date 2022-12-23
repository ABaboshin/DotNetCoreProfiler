using CommandLine;
using Interception.Attributes;
using Interception.Core.Info;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using TypeInfo = Interception.Core.Info.TypeInfo;

namespace Interception.Generator
{
    class Program
    {
        static void Main(string[] args)
        {
            Parser.Default.ParseArguments<Options>(args)
                .WithParsed(opts => {

                    var strict = new List<StrictInterceptionInfo>();
                    //var methodFinders = new List<MethodFinderInfo>();
                    //var attributed = new List<AttributedInterceptor>();
                    var skipAssemblies = File.Exists(opts.Skip) ?  File.ReadAllLines(opts.Skip) : new string[] { };
                    // var enabledAssemblies = File.Exists(opts.EnabledAssemblies) ? File.ReadAllLines(opts.EnabledAssemblies) : new string[] { };
                    LoaderInfo loader = null;
                                        DefaultInitializerInfo defaultInitializer = null;


                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        var fullPath = Path.Combine(opts.Path, new FileInfo(assemblyPath).Name);
                        strict.AddRange(ProcessStrictInterceptors(assembly, fullPath));
                        //methodFinders.AddRange(ProcessMethodFinders(assembly));
                        //attributed.AddRange(ProcessAttributedInterceptors(assembly));
                        if (loader is null)
                        {
                            loader = FindLoader(assembly, fullPath);
                        }
                        else if (FindLoader(assembly, fullPath) != null)
                        {
                                                        throw new Exception("duplicate loader");
                        }

                        if (defaultInitializer is null)
                        {
                            defaultInitializer = FindDefaultInitializer(assembly, fullPath);
                        }
                        else if (FindDefaultInitializer(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate default initializer");

                        }
                    }

                    if (loader is null)
                    {
                                                throw new Exception("load not found");
                    }

                    if (defaultInitializer is null)
                    {
                        throw new Exception("default initializer not found");

                    }

                    var result = new ProfilerInfo{
                        Assemblies = opts.Assemblies.Select(a => Path.Combine(opts.Path, new FileInfo(a).Name)).ToList(),
                        Loader = loader,
                        SkipAssemblies = skipAssemblies?.OrderBy(s => s).ToList(),
                        // enabledAssemblies = enabledAssemblies?.OrderBy(s => s),
                        Strict = strict,
                        DefaultInitializer = defaultInitializer
                        //loader = opts.Loader
                    };

                    File.WriteAllText(opts.Output, JsonConvert.SerializeObject(result, Formatting.Indented));
                });
        }

                private static DefaultInitializerInfo FindDefaultInitializer(Assembly assembly, string path)
        {
            var result = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<DefaultInitializerAttribute>().Any())
                .SelectMany(type => type.GetCustomAttributes<DefaultInitializerAttribute>().Select(attribute => new { type, attribute }))
                .Select(info => {
                    return new DefaultInitializerInfo
                    {
                        TypeName = info.type.FullName,
                        AssemblyPath = path
                    };
                })
                .FirstOrDefault();

            return result;
        }


        //private static IEnumerable<AttributedInterceptor> ProcessAttributedInterceptors(Assembly assembly)
        //{
        //    var result = assembly
        //        .GetTypes()
        //        .Where(type => type.GetCustomAttributes<MethodInterceptorImplementationAttribute>().Any())
        //        .SelectMany(type => type.GetCustomAttributes<MethodInterceptorImplementationAttribute>().Select(attribute => new { type, attribute }))
        //        .Select(info => {
        //            return new AttributedInterceptor
        //            {
        //                AttributeType = info.attribute.MethodInterceptorAttribute.FullName,
        //                Interceptor = new TypeInfo
        //                {
        //                    AssemblyName = assembly.GetName().Name,
        //                    TypeName = info.type.FullName
        //                }
        //            };
        //        })
        //        .ToList();

        //    return result;
        //}

        private static LoaderInfo FindLoader(Assembly assembly, string path)
        {
            var result = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<LoaderAttribute>().Any())
                .SelectMany(type => type.GetCustomAttributes<LoaderAttribute>().Select(attribute => new { type, attribute }))
                .Select(info => {
                    return new LoaderInfo
                    {
                        TypeName = info.type.FullName,
                        AssemblyPath = path
                    };
                })
                .FirstOrDefault();

            return result;
        }

        private static List<StrictInterceptionInfo> ProcessStrictInterceptors(Assembly assembly, string path)
        {
            var interceptors = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Any())
                .SelectMany(type => type.GetCustomAttributes().Where(c => typeof(StrictInterceptAttribute).IsAssignableFrom(c.GetType())).Select(attribute => new { type, attribute = (StrictInterceptAttribute)attribute }))
                .Select(info =>
                {
                    return new StrictInterceptionInfo
                    {
                        // IgnoreCallerAssemblies = info.attribute.IgnoreCallerAssemblies?.OrderBy(a => a).Distinct().ToArray(),
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
                        },
                        AssemblyPath = path
                    };
                })
                .ToList();

            return interceptors;
        }

        //private static List<MethodFinderInfo> ProcessMethodFinders(Assembly assembly)
        //{
        //    var interceptors = assembly
        //        .GetTypes()
        //        .Where(type => type.GetCustomAttributes().Where(c => typeof(MethodFinderAttribute).IsAssignableFrom(c.GetType())).Any())
        //        .SelectMany(type => type.GetCustomAttributes().Where(c => typeof(MethodFinderAttribute).IsAssignableFrom(c.GetType())).Select(attribute => new { type, attribute = (MethodFinderAttribute)attribute }))
        //        .Select(info =>
        //        {
        //            return new MethodFinderInfo
        //            {
        //                Target = new TargetMethod
        //                {
        //                    AssemblyName = info.attribute.TargetAssemblyName,
        //                    MethodName = info.attribute.TargetMethodName,
        //                    TypeName = info.attribute.TargetTypeName,
        //                    MethodParametersCount = info.attribute.TargetMethodParametersCount,
        //                },
        //                MethodFinder = new TypeInfo
        //                {
        //                    AssemblyName = info.type.Assembly.GetName().Name,
        //                    TypeName = info.type.FullName,
        //                }
        //            };
        //        })
        //        .ToList();

        //    return interceptors;
        //}
    }

    
}
