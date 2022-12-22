using Interception.Attributes;
using Interception.Core;
using Interception.Core.Info;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace interceptor
{
    [Loader]
    public class DefaultLoader
    {
        static DefaultLoader()
        {
            // TODO we have already this information in the CorProfiler
            // bypass instead of loading twice
            var configuartionFile = Environment.GetEnvironmentVariable("PROFILER_CONFIGURATION");
            var configuration = JsonConvert.DeserializeObject<ProfilerInfo>(File.ReadAllText(configuartionFile));

            foreach (var interceptor in configuration.Strict.GroupBy(x => x.AssemblyPath).Select(x => new { Path = x.Key, Type = x.First().Interceptor.TypeName }))
            {
                Console.WriteLine($"Load {interceptor.Path}");
                var a = Assembly.LoadFile(interceptor.Path);
                Console.WriteLine($"Loaded {a.FullName} {a.GetName().Version} create {interceptor.Type}");
                a.CreateInstance(interceptor.Type);
                Console.WriteLine($"create {interceptor.Type} done");
            }
        }
    }
}
