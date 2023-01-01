﻿using Interception.Attributes.Tracing;
using Interception.Tracing.Extensions;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Xml.Linq;

namespace Interception.Tracing.Impl
{
    public class TracingImplementation
    {
        protected static AsyncLocal<IScope> _scope = new AsyncLocal<IScope>();

        [TracingBeginMethod]
        public static void BeginTracing(string name)
        {
            Console.WriteLine($"begin tracing {name}");
            try
            {
                var parentSpanContext = GlobalTracer.Instance.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(new Dictionary<string, string>()));
                var spanBuilder = GlobalTracer.Instance
                        .BuildSpan(name)
                        .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                        .AsChildOf(parentSpanContext);

                _scope.Value = spanBuilder.StartActive(true);
            }
            catch (Exception)
            {
            }
        }

        [TracingAddParameterMethod]
        public static void AddParameter<T>(string name, T value)
        {
            Console.WriteLine($"AddParameter {name} {value}");
            try
            {
                _scope.Value.Span.SetTag(name, value.ToString());
            }
            catch (Exception)
            {
            }
        }

        [TracingEndMethod]
        public static void EndTracing<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"EndTracing {result} {ex}");
            try
            {
                if (_scope.Value != null)
                {
                    if (ex != null)
                    {
                        _scope.Value.Span.SetException(ex);
                    }

                    _scope.Value.Span.SetTag("result", result?.ToString());

                    _scope.Value.Dispose();
                    _scope.Value = null;
                }
            }
            catch (Exception)
            {
            }
        }
    }
}