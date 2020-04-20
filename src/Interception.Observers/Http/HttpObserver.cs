using Interception.Common.Extensions;
using Interception.Observers.Configuration;
using Interception.Tracing.Extensions;
using Microsoft.AspNetCore.Http;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace Interception.Observers.Http
{
    /// <summary>
    /// http request diagnsotic events observer
    /// </summary>
    public class HttpObserver : IObserver<KeyValuePair<string, object>>
    {
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
            var exception = (Exception)value.GetType().GetTypeInfo().GetDeclaredProperty("exception")?.GetValue(value);
            Tracing.Tracing.CurrentScope.Span.SetException(exception);
        }

        /// <summary>
        /// process request finished
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStopEvent(object value)
        {
            var httpContext = (HttpContext)value.GetType().GetTypeInfo().GetDeclaredProperty("HttpContext")?.GetValue(value);
            
            Tracing.Tracing.CurrentScope.Span
                    .SetTag(Tags.HttpStatus, httpContext.Response.StatusCode);

            Tracing.Tracing.CurrentScope.Dispose();
        }

        /// <summary>
        /// process before action => extract binding information
        /// </summary>
        /// <param name="value"></param>
        private void ProcessBeforeAction(object value)
        {
            var httpContext = value.GetType().GetTypeInfo().GetDeclaredProperty("httpContext")?.GetValue(value);
            var actionDescriptor = value.GetType().GetTypeInfo().GetDeclaredProperty("actionDescriptor")?.GetValue(value);

            actionDescriptor.TryGetPropertyValue("ActionName", out string actionName);
            actionDescriptor.TryGetPropertyValue("ControllerName", out string controllerName);

            Tracing.Tracing.CurrentScope.Span
                .SetTag("ActionName", actionName)
                .SetTag("ControllerName", controllerName);
        }

        /// <summary>
        /// process request started event
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStartEvent(object value)
        {
            var httpContext = (HttpContext)value.GetType().GetTypeInfo().GetDeclaredProperty("HttpContext")?.GetValue(value);

            var extracted = Tracing.Tracing.Tracer
                        .Extract(BuiltinFormats.HttpHeaders, new RequestHeadersExtractAdapter(httpContext));

            Tracing.Tracing.CurrentScope = Tracing.Tracing.Tracer
                .BuildSpan("http")
                .WithTag("traceIdentifier", httpContext.TraceIdentifier)
                .AsChildOf(extracted)
                .StartActive();
        }
    }
}
