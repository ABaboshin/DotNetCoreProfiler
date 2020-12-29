using Interception.OpenTracing.Propagation;
using Interception.Tracing;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Util;

namespace Interception.OpenTracing
{
    public class Tracer : ITracer
    {
        private readonly ServiceConfiguration _serviceConfiguration;
        private readonly string _spanContextKey;
        private readonly IMetricsSender _metricSender;

        public Tracer(ServiceConfiguration serviceConfiguration, string spanContextKey, IMetricsSender metricSender)
        {
            _serviceConfiguration = serviceConfiguration;
            _spanContextKey = spanContextKey;
            _metricSender = metricSender;
            ScopeManager = new AsyncLocalScopeManager();
        }

        public IScopeManager ScopeManager { get; }

        public ISpan ActiveSpan => ScopeManager.Active?.Span;

        public ISpanBuilder BuildSpan(string operationName)
        {
            return new SpanBuilder(this, operationName);
        }

        public ISpanContext Extract<TCarrier>(IFormat<TCarrier> format, TCarrier carrier)
        {
            // TODO only ITextMap is supported currently
            var extractor = new TextMapExtractor(_spanContextKey);
            return extractor.Extract((ITextMap)carrier);
        }

        public void Inject<TCarrier>(ISpanContext spanContext, IFormat<TCarrier> format, TCarrier carrier)
        {
            // TODO only ITextMap is supported currently
            var injector = new TextMapInjector(_spanContextKey);
            injector.Inject(spanContext, (ITextMap)carrier);
        }

        internal void ReportSpan(Span span)
        {
            _metricSender.Histogram(span);
        }
    }
}
