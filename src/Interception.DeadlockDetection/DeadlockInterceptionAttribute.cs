using Interception.Attributes;
using System;

namespace Interception.DeadlockDetection
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class DeadlockInterceptionAttribute : StrictInterceptAttribute
    {
        public DeadlockInterceptionAttribute()
        {
            IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.Authentication.Core",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Hosting",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Http",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Pipelines.Sockets.Unofficial",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "Serilog.Sinks.Console",
        "StackExchange.Redis",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
        "System.IO.Compression",
        "System.IO.FileSystem.Watcher",
        "System.IO.Pipelines",
        "System.Linq.Expressions",
        "System.Net.Http",
        "System.Net.NameResolution",
        "System.Net.Security",
        "System.Net.Sockets",
        "System.Private.Uri",
        "System.Private.Xml",
        "System.Private.Xml.Linq",
        "System.Reflection.Metadata",
        "System.Security.Cryptography.X509Certificates",
        "System.Text.Encoding.CodePages",
        "System.Text.RegularExpressions",
        "System.Threading.Channels" };
        }
    }
}
