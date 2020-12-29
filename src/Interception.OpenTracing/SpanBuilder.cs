using Microsoft.AspNetCore.Server.Kestrel.Core.Internal.Infrastructure;
using OpenTracing;
using OpenTracing.Tag;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.OpenTracing
{
    public class SpanBuilder : ISpanBuilder
    {
        public Tracer Tracer { get; }
        private readonly string _operationName;

        private readonly List<Reference> _references = new List<Reference>();
        private readonly Dictionary<string, object> _tags = new Dictionary<string, object>();

        private bool _ignoreActiveSpan = false;
        private DateTime? _startTimestampUtc;

        public SpanBuilder(Tracer tracer, string operationName)
        {
            Tracer = tracer;
            _operationName = operationName;
        }

        public ISpanBuilder AddReference(string referenceType, ISpanContext referencedContext)
        {
            if (referencedContext is null)
            {
                return this;
            }

            _references.Add(new Reference(referencedContext, referenceType));

            return this;
        }

        public ISpanBuilder AsChildOf(ISpanContext parent) => AddReference(References.ChildOf, parent);

        public ISpanBuilder AsChildOf(ISpan parent) => AsChildOf(parent?.Context);

        public ISpanBuilder IgnoreActiveSpan()
        {
            _ignoreActiveSpan = true;
            return this;
        }

        public ISpan Start()
        {
            if (!_ignoreActiveSpan && !_references.Any())
            {
                AsChildOf(Tracer.ActiveSpan);
            }

            SpanContext spanContext = null;

            if (!_references.Any())
            {
                var part1 = CorrelationIdGenerator.GetNextId();
                var part2 = CorrelationIdGenerator.GetNextId();

                spanContext = new SpanContext(part1, part2, "", new Dictionary<string, string>());
            }
            else
            {
                var reference = _references.Where(r => r.Type == References.ChildOf).FirstOrDefault();
                var baggage = reference.Context.GetBaggageItems();

                spanContext = new SpanContext(reference.Context.TraceId, CorrelationIdGenerator.GetNextId(), reference.Context.SpanId, baggage.ToDictionary(b => b.Key, b => b.Value));
            }

            if (_startTimestampUtc is null)
            {
                _startTimestampUtc = DateTime.UtcNow;
            }

            var span = new Span(this, _operationName, spanContext, _startTimestampUtc.Value, _tags, _references);
            return span;
        }

        public IScope StartActive()
        {
            return StartActive(finishSpanOnDispose: true);
        }

        public IScope StartActive(bool finishSpanOnDispose)
        {
            return Tracer.ScopeManager.Activate(Start(), finishSpanOnDispose);
        }

        public ISpanBuilder WithStartTimestamp(DateTimeOffset timestamp)
        {
            _startTimestampUtc = timestamp.UtcDateTime;
            return this;
        }

        public ISpanBuilder WithTag(string key, string value)
        {
            _tags[key] = value;
            return this;
        }

        public ISpanBuilder WithTag(string key, bool value)
        {
            _tags[key] = value;
            return this;
        }

        public ISpanBuilder WithTag(string key, int value)
        {
            _tags[key] = value;
            return this;
        }

        public ISpanBuilder WithTag(string key, double value)
        {
            _tags[key] = value;
            return this;
        }

        public ISpanBuilder WithTag(BooleanTag tag, bool value)
        {
            _tags[tag.Key] = value;
            return this;
        }

        public ISpanBuilder WithTag(IntOrStringTag tag, string value)
        {
            _tags[tag.Key] = value;
            return this;
        }

        public ISpanBuilder WithTag(IntTag tag, int value)
        {
            _tags[tag.Key] = value;
            return this;
        }

        public ISpanBuilder WithTag(StringTag tag, string value)
        {
            _tags[tag.Key] = value;
            return this;
        }
    }
}
