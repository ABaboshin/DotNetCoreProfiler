using Interception.Attributes;
using Interception.Core.Info;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Interception.Core
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
                Assembly.LoadFile(interceptor.Path).CreateInstance(interceptor.Type);
            }
        }
    }
}
