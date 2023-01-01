using CommandLine;
using Interception.Attributes;
using Interception.Attributes.Tracing;
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
                    TypeInfo loader = null;
                    InterceptorMethodInfo defaultInitializer = null;
                    InterceptorMethodInfo exceptionLogger = null;
                    InterceptorMethodInfo tracingBeginMethod = null;
                    InterceptorMethodInfo tracingEndMethod = null;
                    InterceptorMethodInfo tracingAddParameterMethod = null;

                    var traceFileContent = File.Exists(opts.TraceFile) ? File.ReadAllText(opts.TraceFile) : null;
                    var traces = !string.IsNullOrEmpty(traceFileContent) ? System.Text.Json.JsonSerializer.Deserialize<List<TraceMethodInfo>>(traceFileContent) : null;

                    foreach (var assemblyPath in opts.Assemblies)
                    {
                        var assembly = Assembly.LoadFrom(assemblyPath);
                        var fullPath = Path.Combine(opts.Path, new FileInfo(assemblyPath).Name);
                        strict.AddRange(ProcessStrictInterceptors(assembly, fullPath));

                        if (loader is null)
                        {
                            loader = FindTypeInfo<LoaderAttribute>(assembly, fullPath);
                        }
                        else if (FindTypeInfo<LoaderAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate loader");
                        }

                        if (defaultInitializer is null)
                        {
                            defaultInitializer = FindInterceptorMethod<DefaultInitializerAttribute>(assembly, fullPath);
                        }
                        else if (FindInterceptorMethod<DefaultInitializerAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate default initializer");
                        }

                        if (exceptionLogger is null)
                        {
                            exceptionLogger = FindInterceptorMethod<ExceptionLoggerAttribute>(assembly, fullPath);
                        }
                        else if (FindInterceptorMethod<ExceptionLoggerAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate default exception logger");
                        }

                        if (tracingBeginMethod is null)
                        {
                            tracingBeginMethod = FindInterceptorMethod<TracingBeginMethodAttribute>(assembly, fullPath);
                        }
                        else if (FindInterceptorMethod<TracingBeginMethodAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate tracing begin method");
                        }

                        if (tracingEndMethod is null)
                        {
                            tracingEndMethod = FindInterceptorMethod<TracingEndMethodAttribute>(assembly, fullPath);
                        }
                        else if (FindInterceptorMethod<TracingEndMethodAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate tracing end method");
                        }

                        if (tracingAddParameterMethod is null)
                        {
                            tracingAddParameterMethod = FindInterceptorMethod<TracingAddParameterMethodAttribute>(assembly, fullPath);
                        }
                        else if (FindInterceptorMethod<TracingAddParameterMethodAttribute>(assembly, fullPath) != null)
                        {
                            throw new Exception("duplicate tracing add parameter method");
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
                        Path = opts.Path,
                        TracingBeginMethod = tracingBeginMethod,
                        TracingEndMethod = tracingEndMethod,
                        TracingAddParameterMethod= tracingAddParameterMethod,
                        Traces = traces
                    };

                    File.WriteAllText(opts.Output, System.Text.Json.JsonSerializer.Serialize(result, new JsonSerializerOptions { 
                        WriteIndented = true,
                    }));
                });
        }

        private static InterceptorMethodInfo FindInterceptorMethod<T>(Assembly assembly, string path) where T : Attribute
        {
            var result = assembly
                .GetTypes()
                .SelectMany(t => t.GetMethods())
                .Where(m => m.GetCustomAttributes<T>().Any())
                .SelectMany(m => m.GetCustomAttributes<T>().Select(attribute => new { m, attribute }))
                .Select(info => {
                    return new InterceptorMethodInfo
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

        private static TypeInfo FindTypeInfo<T>(Assembly assembly, string path) where T : Attribute
        {
            var result = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<T>().Any())
                .SelectMany(type => type.GetCustomAttributes<T>().Select(attribute => new { type, attribute }))
                .Select(info => {
                    return new TypeInfo
                    {
                        TypeName = info.type.FullName,
                        AssemblyPath = path,
                        AssemblyName = assembly.GetName().Name
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
                        Target = new TargetMethodnfo
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
                            AssemblyPath = path
                        },
                        Priority = info.attribute.Priority
                    };
                })
                .ToList();

            return interceptors;
        }
    }
}
