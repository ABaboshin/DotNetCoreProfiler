using Interception.Common;
using Serilog;
using Serilog.Core;

namespace Interception.Serilog
{
    public static class SerilogInterceptor
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "Serilog", TargetMethodName = "CreateLogger", TargetTypeName = "Serilog.LoggerConfiguration", TargetMethodParametersCount = 0)]
        public static object CreateLogger(object loggerConfiguration, int mdToken, long moduleVersionPtr)
        {
            var config = (LoggerConfiguration)loggerConfiguration;
            config.Enrich.With((ILogEventEnricher)new SerilogEnricher());

            return MethodExecutor.ExecuteMethod(loggerConfiguration, new object[] { }, mdToken, moduleVersionPtr, true);
        }
    }
}
