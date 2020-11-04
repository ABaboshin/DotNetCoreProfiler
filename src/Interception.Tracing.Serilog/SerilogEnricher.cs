using OpenTracing.Util;
using Serilog.Core;
using Serilog.Events;

namespace Interception.Tracing.Serilog
{
    public class SerilogEnricher :
        ILogEventEnricher
    {
        public void Enrich(LogEvent logEvent, ILogEventPropertyFactory factory)
        {
            if (GlobalTracer.Instance.ActiveSpan is null)
            {
                return;
            }

            var traceId = GlobalTracer.Instance.ActiveSpan.Context.TraceId;

            if (!string.IsNullOrEmpty(traceId))
                logEvent.AddOrUpdateProperty(factory.CreateProperty("TraceId", traceId, true));
        }
    }
}
