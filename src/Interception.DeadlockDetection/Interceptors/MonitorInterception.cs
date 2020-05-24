using Interception.Attributes;
using Interception.DeadlockDetection.Enhancements;
using System;

namespace Interception.DeadlockDetection.Interceptors
{
    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "Enter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 1)]
    public class MonitorEnterObjectInterceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var lockTaken = false;
            MonitorEnhancements.Enter(GetParameter(0), ref lockTaken);
        }
    }

    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "Enter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 2)]
    public class MonitorEnterObjectBoolInterceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var lockTaken = false;
            MonitorEnhancements.Enter(GetParameter(0), ref lockTaken);
            ModifyParameter(1, lockTaken);
        }
    }

    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "TryEnter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 1)]
    public class MonitorTryEnter1Interceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var lockTaken = false;
            MonitorEnhancements.TryEnter(GetParameter(0), TimeSpan.FromMilliseconds(0.0), ref lockTaken);
            Result = lockTaken;
        }
    }

    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "TryEnter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 2)]
    public class MonitorTryEnter2Interceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var lockTaken = false;

            var timeout = TimeSpan.FromMilliseconds(0.0);

            if (GetParameter(1) is int)
            {
                timeout = TimeSpan.FromMilliseconds((int)GetParameter(1));
            }

            if (GetParameter(1) is TimeSpan)
            {
                timeout = (TimeSpan)GetParameter(1);
            }

            MonitorEnhancements.TryEnter(GetParameter(0), timeout, ref lockTaken);

            if (GetParameter(1) is bool)
            {
                ModifyParameter(1, lockTaken);
            }

            Result = lockTaken;
        }
    }

    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "TryEnter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 3)]
    public class MonitorTryEnter3Interceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            var lockTaken = false;

            var timeout = TimeSpan.FromMilliseconds(0.0);

            if (GetParameter(1) is int)
            {
                timeout = TimeSpan.FromMilliseconds((int)GetParameter(1));
            }

            if (GetParameter(1) is TimeSpan)
            {
                timeout = (TimeSpan)GetParameter(1);
            }

            MonitorEnhancements.TryEnter(GetParameter(0), timeout, ref lockTaken);

            ModifyParameter(2, lockTaken);
        }
    }

    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "GreenPipes",
        "Interception.DeadlockDetection",
        "Interception.OpenTracing.Prometheus",
        "Jaeger",
        "MassTransit",
        "Microsoft.AspNetCore.DataProtection",
        "Microsoft.AspNetCore.HostFiltering",
        "Microsoft.AspNetCore.Server.Kestrel.Core",
        "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets",
        "Microsoft.EntityFrameworkCore",
        "Microsoft.Extensions.Caching.Memory",
        "Microsoft.Extensions.DependencyInjection",
        "Microsoft.Extensions.FileProviders.Physical",
        "Microsoft.Extensions.Logging",
        "Microsoft.Extensions.Primitives",
        "OpenTracing",
        "Quartz",
        "RabbitMQ.Client",
        "Remotion.Linq",
        "StackExchange.Redis.StrongName",
        "System.Collections.Concurrent",
        "System.ComponentModel.Primitives",
        "System.ComponentModel.TypeConverter",
        "System.Configuration.ConfigurationManager",
        "System.Data.SqlClient",
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
        "System.Text.RegularExpressions" },
        TargetAssemblyName = "System.Threading", TargetMethodName = "Enter", TargetTypeName = "System.Threading.Monitor", TargetMethodParametersCount = 1)]
    public class MonitorExitObjectInterceptor : DeadlockInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            MonitorEnhancements.Exit(GetParameter(0));
        }
    }
}
