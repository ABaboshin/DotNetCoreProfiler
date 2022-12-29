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
        }
    }
}
