using Interception.Common.Extensions;
using Interception.Observers.Configuration;
using Interception.Tracing;
using Interception.Tracing.Extensions;
using Microsoft.AspNetCore.Http;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using System;
using System.Collections.Generic;

namespace Interception.Observers
{
    /// <summary>
    /// asp net core diagnsotic events observer
    /// </summary>
    public class AspNetCoreDiagnostics : IObserver<KeyValuePair<string, object>>
    {
        public static readonly string TraceHeaderName = "X-Trace-Id";
        private readonly AspNetCoreConfiguration _configuration;

        public AspNetCoreDiagnostics(AspNetCoreConfiguration configuration)
        {
            _configuration = configuration;
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

            if (kv.Key == "Microsoft.AspNetCore.Hosting.UnhandledException")
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
            if (value.TryGetPropertyValue("exception", out Exception exception))
            {
                Tracing.Tracing.CurrentScope.Span.SetException(exception);
            }
        }

        /// <summary>
        /// process request finished
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStopEvent(object value)
        {
            if (value.TryGetPropertyValue("HttpContext", out HttpContext httpContext))
            {
                Tracing.Tracing.CurrentScope.Span
                    .SetTag(Tags.HttpStatus, httpContext.Response.StatusCode);

                Tracing.Tracing.CurrentScope.Dispose();
            }
        }

        /// <summary>
        /// process before action => extract binding information
        /// </summary>
        /// <param name="value"></param>
        private void ProcessBeforeAction(object value)
        {
            if (value.TryGetPropertyValue("actionDescriptor", out object actionDescriptor))
            {
                actionDescriptor.TryGetPropertyValue("ActionName", out string actionName);
                actionDescriptor.TryGetPropertyValue("ControllerName", out string controllerName);

                Tracing.Tracing.CurrentScope.Span
                    .SetTag("ActionName", actionName)
                    .SetTag("ControllerName", controllerName);
            }
        }

        /// <summary>
        /// process request started event
        /// </summary>
        /// <param name="value"></param>
        private void ProcessStartEvent(object value)
        {
            if (value.TryGetPropertyValue("HttpContext", out HttpContext httpContext))
            {
                var extracted = Tracing.Tracing.Tracer
                        .Extract(BuiltinFormats.HttpHeaders, new RequestHeadersExtractAdapter(httpContext));

                Tracing.Tracing.CurrentScope = Tracing.Tracing.Tracer
                    .BuildSpan(_configuration.Name)
                    .WithTag(Constants.TraceIdentifier, httpContext.TraceIdentifier)
                    .AsChildOf(extracted)
                    .StartActive();

                httpContext.Response.Headers.Add(TraceHeaderName, Tracing.Tracing.CurrentScope.Span.Context.TraceId);
            }
        }
    }
}
