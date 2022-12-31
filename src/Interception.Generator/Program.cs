using CommandLine;
using Interception.Attributes;
using Interception.Core.Info;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text.Json;
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
                    var skipAssemblies = File.Exists(opts.Skip) ? File.ReadAllLines(opts.Skip) : new string[] { };
                    LoaderInfo loader = null;
                    DefaultInitializerInfo defaultInitializer = null;
                    ExceptionLoggerInfo exceptionLogger = null;

                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        var fullPath = Path.Combine(opts.Path, new FileInfo(assemblyPath).Name);
                        strict.AddRange(ProcessStrictInterceptors(assembly, fullPath));

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

                        if (exceptionLogger is null)
                        {
                            exceptionLogger = FindExceptionLogger(assembly, fullPath);
                        }
                        else if (FindExceptionLogger(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate default exception logger");
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

                    var result = new ProfilerInfo
                    {
                        Loader = loader,
                        SkipAssemblies = skipAssemblies?.OrderBy(s => s).ToList(),
                        Strict = strict,
                        DefaultInitializer = defaultInitializer,
                        ExceptionLogger = exceptionLogger,
                        Path = opts.Path
                    };

                    File.WriteAllText(opts.Output, System.Text.Json.JsonSerializer.Serialize(result, new JsonSerializerOptions { 
                        WriteIndented = true,
                    }));
                });
        }

        private static DefaultInitializerInfo FindDefaultInitializer(Assembly assembly, string path)
        {
            var result = assembly
                .GetTypes()
                .SelectMany(t => t.GetMethods())
                .Where(m => m.GetCustomAttributes<DefaultInitializerAttribute>().Any())
                .SelectMany(m => m.GetCustomAttributes<DefaultInitializerAttribute>().Select(attribute => new { m, attribute }))
                .Select(info => {
                    return new DefaultInitializerInfo
                    {
                        MethodName = info.m.Name,
                        TypeName = info.m.DeclaringType.FullName,
                        AssemblyPath = path,
                        AssemblyName = assembly.GetName().Name,
                    };
                })
                .FirstOrDefault();

            return result;
        }

        private static ExceptionLoggerInfo FindExceptionLogger(Assembly assembly, string path)
        {
            var result = assembly
                .GetTypes()
                .SelectMany(t => t.GetMethods())
                .Where(m => m.GetCustomAttributes<ExceptionLoggerAttribute>().Any())
                .SelectMany(m => m.GetCustomAttributes<ExceptionLoggerAttribute>().Select(attribute => new { m, attribute }))
                .Select(info => {
                    return new ExceptionLoggerInfo
                    {
                        MethodName = info.m.Name,
                        TypeName = info.m.DeclaringType.FullName,
                        AssemblyPath = path,
                        AssemblyName = assembly.GetName().Name,
                    };
                })
                .FirstOrDefault();

            return result;
        }

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
                        AssemblyPath = path,
                        Priority = info.attribute.Priority
                    };
                })
                .ToList();

            return interceptors;
        }
    }
}
