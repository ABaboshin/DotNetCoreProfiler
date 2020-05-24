using Interception.Attributes;
using Interception.DeadlockDetection.Enhancements;
using System;

namespace Interception.DeadlockDetection.Interceptors
{
    [StrictIntercept(
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
        IgnoreCallerAssemblies = new string[] { "System.Reflection.Metadata", "System.Net.Sockets", "System.ComponentModel.TypeConverter", "Microsoft.Extensions.Logging", "Microsoft.Extensions.DependencyInjection", "System.Collections.Concurrent", "System.IO.FileSystem.Watcher", "Microsoft.Extensions.FileProviders.Physical", "System.Data.SqlClient", "System.ComponentModel.Primitives", "Microsoft.EntityFrameworkCore", "System.Linq.Expressions", "Microsoft.Extensions.Primitives", "System.Text.RegularExpressions", "System.Private.Xml.Linq", "System.Private.Xml", "RabbitMQ.Client", "MassTransit", "GreenPipes", "System.Private.Uri", "Microsoft.Extensions.Caching.Memory", "Remotion.Linq", "Jaeger", "System.Text.Encoding.CodePages", "System.Net.Security", "System.Text.Encoding.CodePages", "System.Security.Cryptography.X509Certificates", "OpenTracing", "Microsoft.AspNetCore.Server.Kestrel.Core", "System.IO.Pipelines", "System.Net.Http", "Quartz", "System.Configuration.ConfigurationManager", "StackExchange.Redis.StrongName", "Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets", "Microsoft.AspNetCore.HostFiltering", "Interception.DeadlockDetection" },
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
