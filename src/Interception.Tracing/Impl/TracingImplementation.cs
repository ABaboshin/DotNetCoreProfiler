using Interception.Attributes.Tracing;
using Interception.Tracing.Extensions;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;

namespace Interception.Tracing.Impl
{
    public class TracingImplementation
    {
        protected static AsyncLocal<IScope> _scope = new AsyncLocal<IScope>();

        [TracingBeginMethod]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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

                    if (result != null && result is Task)
                    {
                        var task = result as Task;
                        if (!task.IsCanceled && !task.IsCompleted)
                        {
                            task.GetAwaiter().GetResult();
                        }
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
