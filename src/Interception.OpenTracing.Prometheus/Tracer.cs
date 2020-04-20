using Interception.OpenTracing.Prometheus.Propagation;
using Interception.Tracing;
using Microsoft.Extensions.Logging;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Util;
using System;

namespace Interception.OpenTracing.Prometheus
{
    public class Tracer : ITracer
    {
        private readonly ILoggerFactory _loggerFactory;
        private readonly ServiceConfiguration _serviceConfiguration;
        private readonly string _spanContextKey;

        public Tracer(ILoggerFactory loggerFactory, ServiceConfiguration serviceConfiguration, string spanContextKey)
        {
            _loggerFactory = loggerFactory;
            _serviceConfiguration = serviceConfiguration;
            _spanContextKey = spanContextKey;
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
            Console.WriteLine($"Inject");
            // TODO only ITextMap is supported currently
            var injector = new TextMapInjector(_spanContextKey);
            injector.Inject(spanContext, (ITextMap)carrier);
        }

        internal void ReportSpan(Span span)
        {
            MetricsSender.Histogram(span);
        }
    }
}
