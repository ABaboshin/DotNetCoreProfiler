using Serilog.Core;
using Serilog.Events;

namespace Interception.Serilog
{
    public class SerilogEnricher :
        ILogEventEnricher
    {
        public void Enrich(LogEvent logEvent, ILogEventPropertyFactory factory)
        {
            if (Tracing.Tracing.CurrentScope is null)
            {
                return;
            }

            var traceId = Tracing.Tracing.CurrentScope?.Span.Context.TraceId;

            if (!string.IsNullOrEmpty(traceId))
                logEvent.AddOrUpdateProperty(factory.CreateProperty("TraceId", traceId, true));
        }
    }
}
