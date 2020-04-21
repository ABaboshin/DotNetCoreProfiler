using OpenTracing;
using OpenTracing.Tag;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing.Prometheus
{
    internal class Span : ISpan
    {
        private SpanBuilder _spanBuilder;
        public string OperationName { get; private set; }
        public SpanContext Context { get; private set; }
        private DateTime _startTimestampUtc;
        private IDictionary<string, object> _tags;
        private List<Reference> _references;

        private readonly object _lock = new object();

        private DateTime? _finishTimestampUtc;

        public Span(SpanBuilder spanBuilder, string operationName, SpanContext spanContext, DateTime startTimestampUtc, IDictionary<string, object> tags, List<Reference> references)
        {
            _spanBuilder = spanBuilder;
            OperationName = operationName;
            Context = spanContext;
            _startTimestampUtc = startTimestampUtc;
            _tags = tags;
            _references = references;
        }

        ISpanContext ISpan.Context => Context;

        public IDictionary<string, object> GetTags() => _tags;

        public TimeSpan Duration => _finishTimestampUtc.HasValue ? _finishTimestampUtc.Value - _startTimestampUtc : TimeSpan.Zero;

        public void Finish()
        {
            lock (_lock)
            {
                _finishTimestampUtc = DateTime.UtcNow;
            }

            _spanBuilder.Tracer.ReportSpan(this);
        }

        public void Finish(DateTimeOffset finishTimestamp)
        {
            lock (_lock)
            {
                _finishTimestampUtc = finishTimestamp.UtcDateTime;
            }

            _spanBuilder.Tracer.ReportSpan(this);
        }

        public string GetBaggageItem(string key)
        {
            return Context.GetBaggageItems().Where(b => b.Key == key).Select(b => b.Value).FirstOrDefault();
        }

        public ISpan Log(IEnumerable<KeyValuePair<string, object>> fields)
        {
            return this;
        }

        public ISpan Log(DateTimeOffset timestamp, IEnumerable<KeyValuePair<string, object>> fields)
        {
            return this;
        }

        public ISpan Log(string @event)
        {
            return this;
        }

        public ISpan Log(DateTimeOffset timestamp, string @event)
        {
            return this;
        }

        public ISpan SetBaggageItem(string key, string value)
        {
            Context = Context.WithBaggageItem(key, value);
            return this;
        }

        public ISpan SetOperationName(string operationName)
        {
            lock (_lock)
            {
                OperationName = operationName;
            }

            return this;
        }

        public ISpan SetTag(string key, string value)
        {
            return SetTagInternal(key, value);
        }

        public ISpan SetTag(string key, bool value)
        {
            return SetTagInternal(key, value);
        }

        public ISpan SetTag(string key, int value)
        {
            return SetTagInternal(key, value);
        }

        public ISpan SetTag(string key, double value)
        {
            return SetTagInternal(key, value);
        }

        public ISpan SetTag(BooleanTag tag, bool value)
        {
            return SetTagInternal(tag.Key, value);
        }

        public ISpan SetTag(IntOrStringTag tag, string value)
        {
            return SetTagInternal(tag.Key, value);
        }

        public ISpan SetTag(IntTag tag, int value)
        {
            return SetTagInternal(tag.Key, value);
        }

        public ISpan SetTag(StringTag tag, string value)
        {
            return SetTagInternal(tag.Key, value);
        }

        private ISpan SetTagInternal(string tag, object value)
        {
            lock (_lock)
            {
                _tags[tag] = value;
            }

            return this;
        }
    }
}