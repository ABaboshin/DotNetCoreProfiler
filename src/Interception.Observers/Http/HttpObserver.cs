using Interception.Common.Extensions;
using Interception.Metrics;
using Interception.Metrics.Extensions;
using Interception.Observers.Configuration;
using Interception.Tracing;
using Microsoft.AspNetCore.Http;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;

namespace Interception.Observers.Http
{
    /// <summary>
    /// http request diagnsotic events observer
    /// </summary>
    public class HttpObserver : IObserver<KeyValuePair<string, object>>
    {
        public static AsyncLocal<string> asyncLocal = new AsyncLocal<string>();

        private readonly ConcurrentDictionary<string, RequestInfo> info = new ConcurrentDictionary<string, RequestInfo>();
        private readonly HttpConfiguration _httpConfiguration;

        public HttpObserver(HttpConfiguration httpConfiguration)
        {
            _httpConfiguration = httpConfiguration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(KeyValuePair<string, object> kv)
        {
            if (kv.Key == "Microsoft.AspNetCore.Hosting.HttpRequestIn.Start")
            {
                ProcessStartEvent(kv.Value);
            }

            if (kv.Key == "Microsoft.AspNetCore.Mvc.BeforeAction")
            {
                ProcessBeforeAction(kv.Value);
            }

            if (kv.Key == "Microsoft.AspNetCore.Hosting.HttpRequestIn.Stop")
            {
                ProcessStopEvent(kv.Value);
            }

            if (kv.Key == "Microsoft.AspNetCore.Diagnostics.UnhandledException")
            {
                ProcessUnhandledException(kv.Value);
            }
        }

        /// <summary>
        /// process an unhandled exception
        /// </summary>
        /// <param name="value"></param>
        private void ProcessUnhandledException(object value)
        {
            var httpContext = value.GetType().GetTypeInfo().GetDeclaredProperty("httpContext")?.GetValue(value);
            var exception = value.GetType().GetTypeInfo().GetDeclaredProperty("exception")?.GetValue(value) as Exception;
            if (httpContext != null)
            {
                httpContext.TryGetPropertyValue("TraceIdentifier", out string traceIdentifier);

                Tracing.Tracing.CurrentScope.Span
                    .SetTag(Tags.Error, true)
                    .SetTag(Tags.Error, true)
                    .Log(new Dictionary<string, object>(3)
                    {
                        { LogFields.Event, Tags.Error.Key },
                        { LogFields.ErrorKind, exception.GetType().Name },
                        { LogFields.ErrorObject, exception }
                    });

                if (info.TryGetValue(traceIdentifier, out var existing))
                {
                    info.TryUpdate(traceIdentifier,
                        new RequestInfo
                        {
                            ActionName = existing.ActionName,
                            ControllerName = existing.ControllerName,
                            Start = existing.Start,
                            Exception = exception
                        },
                        existing);
                }
            }
        }

        /// <summary>
        /// process request finished
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStopEvent(object value)
        {
            var httpContext = value.GetType().GetTypeInfo().GetDeclaredProperty("HttpContext")?.GetValue(value);
            if (httpContext != null)
            {
                httpContext.TryGetPropertyValue("TraceIdentifier", out string traceIdentifier);
                httpContext.TryGetPropertyValue("Response", out object response);
                response.TryGetPropertyValue("StatusCode", out object statusCode);

                Tracing.Tracing.CurrentScope.Span
                    .SetTag("StatusCode", statusCode.ToString());

                Tracing.Tracing.CurrentScope.Dispose();

                if (info.TryRemove(traceIdentifier, out var existing))
                {
                    var end = DateTime.UtcNow;
                    var tags = new List<string> {
                            $"action:{existing.ActionName ?? ""}",
                            $"controller:{existing.ControllerName ?? ""}",
                            $"statusCode:{statusCode}",
                            $"traceIdentifier:{traceIdentifier}"
                        };
                    if (existing.Exception != null)
                    {
                        tags.AddRange(existing.Exception.GetTags());
                    }

                    MetricsSender.Histogram(_httpConfiguration.Name,
                        (end - existing.Start).TotalMilliseconds,
                        tags);
                }
            }
        }

        /// <summary>
        /// process before action => extract binding information
        /// </summary>
        /// <param name="value"></param>
        private void ProcessBeforeAction(object value)
        {
            var httpContext = value.GetType().GetTypeInfo().GetDeclaredProperty("httpContext")?.GetValue(value);
            var actionDescriptor = value.GetType().GetTypeInfo().GetDeclaredProperty("actionDescriptor")?.GetValue(value);
            if (httpContext != null)
            {
                httpContext.TryGetPropertyValue("TraceIdentifier", out string traceIdentifier);
                actionDescriptor.TryGetPropertyValue("ActionName", out string actionName);
                actionDescriptor.TryGetPropertyValue("ControllerName", out string controllerName);

                Tracing.Tracing.CurrentScope.Span
                    .SetTag("ActionName", actionName)
                    .SetTag("ActionName", controllerName);

                if (info.TryGetValue(traceIdentifier, out var existing))
                {
                    info.TryUpdate(traceIdentifier,
                        new RequestInfo
                        {
                            ActionName = actionName,
                            ControllerName = controllerName,
                            Start = existing.Start
                        },
                        existing);
                }
            }
        }

        /// <summary>
        /// process request started event
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStartEvent(object value)
        {
            var httpContext = (HttpContext)value.GetType().GetTypeInfo().GetDeclaredProperty("HttpContext")?.GetValue(value);
            if (httpContext != null)
            {
                if (httpContext.TryGetPropertyValue("TraceIdentifier", out string traceIdentifier))
                {
                    info.TryAdd(traceIdentifier, new RequestInfo { Start = DateTime.UtcNow, TraceIdentifier = traceIdentifier });

                    var extracted = Tracing.Tracing.Tracer
                        .Extract(BuiltinFormats.HttpHeaders, new RequestHeadersExtractAdapter(httpContext));

                    Tracing.Tracing.CurrentScope = Tracing.Tracing.Tracer
                        .BuildSpan("http " + Guid.NewGuid().ToString())
                        .AsChildOf(extracted)
                        .StartActive();
                }
            }
        }
    }
}
