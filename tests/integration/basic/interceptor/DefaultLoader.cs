using Interception.Attributes;
using Interception.Core.Info;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace interceptor
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

            foreach (var interceptor in configuration.Strict.GroupBy(x => x.AssemblyPath).Select(x => new { Path = x.Key, Type = x.First().Interceptor.TypeName }))
            {
                Console.WriteLine($"Load interceptor {interceptor.Type}");
                var a = Assembly.LoadFile(interceptor.Path);
                a.CreateInstance(interceptor.Type);
            }

            if (configuration.DefaultInitializer != null)
            {
                Console.WriteLine($"Load default initializer {configuration.DefaultInitializer.TypeName}");
                var a = Assembly.LoadFile(configuration.DefaultInitializer.AssemblyPath);
                a.CreateInstance(configuration.DefaultInitializer.TypeName);
            }

            if (configuration.ExceptionLogger != null)
            {
                Console.WriteLine($"Load exception logger {configuration.ExceptionLogger.TypeName}");
                var a = Assembly.LoadFile(configuration.ExceptionLogger.AssemblyPath);
                a.CreateInstance(configuration.ExceptionLogger.TypeName);
            }
        }
    }


    public class DefaultInitializer
    {
        [DefaultInitializer]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T GetDefault<T>() => default(T);

        static DefaultInitializer() { }
    }

    public class ExceptionLogger
    {
        [ExceptionLogger]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void LogException(Exception exception) { Console.WriteLine("From interceptor " + exception.ToString()); }

        static ExceptionLogger() { }
    }
}
