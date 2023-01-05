using Interception.Attributes;
using Interception.Core.Info;
using System;
using System.IO;
using System.Reflection;

namespace Interception.DefaultImpl
{
    [Loader]
    public class DefaultLoader
    {
        static DefaultLoader()
        {
            //TODO we have already this information in the CorProfiler
            // bypass instead of loading twice
            var configuartionFile = Environment.GetEnvironmentVariable("PROFILER_CONFIGURATION");
            var configuration = System.Text.Json.JsonSerializer.Deserialize<ProfilerInfo>(File.ReadAllText(configuartionFile));

            AppDomain.CurrentDomain.AssemblyResolve += (sender, args) =>
            {
                var assemblyName = new AssemblyName(args.Name);
                var path = Path.Combine(configuration.Path, $"{assemblyName.Name}.dll");

                Console.WriteLine($"Resolve assembly name {assemblyName} to path {path}");

                return Assembly.Load(path);
            };

            foreach (var interceptor in configuration.Strict)
            {
                Console.WriteLine($"Load interceptor {interceptor.Interceptor.TypeName} from {interceptor.Interceptor.AssemblyName}");
                var a = Assembly.Load(interceptor.Interceptor.AssemblyName);
                a.CreateInstance(interceptor.Interceptor.TypeName);
            }

            if (configuration.DefaultInitializer != null)
            {
                Console.WriteLine($"Load default initializer {configuration.DefaultInitializer.TypeName}");
                var a = Assembly.Load(configuration.DefaultInitializer.AssemblyName);
                a.CreateInstance(configuration.DefaultInitializer.TypeName);
            }

            if (configuration.ExceptionLogger != null)
            {
                Console.WriteLine($"Load exception logger {configuration.ExceptionLogger.TypeName}");
                var a = Assembly.Load(configuration.ExceptionLogger.AssemblyName);
                a.CreateInstance(configuration.ExceptionLogger.TypeName);
            }

            if (configuration.TracingBeginMethod != null)
            {
                Console.WriteLine($"Load tracing begin method {configuration.TracingBeginMethod.TypeName}");
                var a = Assembly.Load(configuration.TracingBeginMethod.AssemblyName);
                a.CreateInstance(configuration.TracingBeginMethod.TypeName);
            }

            if (configuration.TracingEndMethod != null)
            {
                Console.WriteLine($"Load tracing end method {configuration.TracingEndMethod.TypeName}");
                var a = Assembly.Load(configuration.TracingEndMethod.AssemblyName);
                a.CreateInstance(configuration.TracingEndMethod.TypeName);
            }

            if (configuration.TracingAddParameterMethod != null)
            {
                Console.WriteLine($"Load tracing add parameter method {configuration.TracingAddParameterMethod.TypeName}");
                var a = Assembly.Load(configuration.TracingAddParameterMethod.AssemblyName);
                a.CreateInstance(configuration.TracingAddParameterMethod.TypeName);
            }

            if (configuration.DebuggerBeginMethod != null)
            {
                Console.WriteLine($"Load debugger begin method {configuration.DebuggerBeginMethod.TypeName}");
                var a = Assembly.Load(configuration.DebuggerBeginMethod.AssemblyName);
                a.CreateInstance(configuration.DebuggerBeginMethod.TypeName);
            }

            if (configuration.DebuggerEndMethod != null)
            {
                Console.WriteLine($"Load debugger end method {configuration.DebuggerEndMethod.TypeName}");
                var a = Assembly.Load(configuration.DebuggerEndMethod.AssemblyName);
                a.CreateInstance(configuration.DebuggerEndMethod.TypeName);
            }

            if (configuration.DebuggerAddParameterMethod != null)
            {
                Console.WriteLine($"Load debugger add parameter method {configuration.DebuggerAddParameterMethod.TypeName}");
                var a = Assembly.Load(configuration.DebuggerAddParameterMethod.AssemblyName);
                a.CreateInstance(configuration.DebuggerAddParameterMethod.TypeName);
            }

            if (configuration.DebuggerInitializerMethod != null)
            {
                Console.WriteLine($"Load debugger initializer {configuration.DebuggerInitializerMethod.TypeName}");
                var a = Assembly.Load(configuration.DebuggerInitializerMethod.AssemblyName);
                var debuggerInitializer = a.CreateInstance(configuration.DebuggerInitializerMethod.TypeName);
                var method = debuggerInitializer.GetType().GetMethod(configuration.DebuggerInitializerMethod.MethodName);
                method.Invoke(debuggerInitializer, new object[] { configuration });
            }
        }
    }
}
