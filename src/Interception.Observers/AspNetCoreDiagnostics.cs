//using Interception.Common.Extensions;
//using Interception.Observers.Configuration;
//using Interception.Tracing;
//using Interception.Tracing.Extensions;
//using Microsoft.AspNetCore.Http;
//using OpenTracing.Propagation;
//using OpenTracing.Tag;
//using OpenTracing.Util;
//using System;
//using System.Collections.Generic;
//using System.Diagnostics;

//namespace Interception.Observers
//{
//    /// <summary>
//    /// asp net core diagnsotic events observer
//    /// </summary>
//    public class AspNetCoreDiagnostics : BaseObserver
//    {
//        private readonly AspNetCoreConfiguration _configuration;

//        public AspNetCoreDiagnostics(AspNetCoreConfiguration configuration)
//        {
//            _configuration = configuration;
//        }

//        public override void OnCompleted()
//        {
//        }

//        public override void OnError(Exception error)
//        {
//        }

//        public override void OnNext(KeyValuePair<string, object> kv)
//        {
//            if (kv.Key == "Microsoft.AspNetCore.Hosting.HttpRequestIn.Start")
//            {
//                ProcessStartEvent(kv.Value);
//            }

//            if (kv.Key == "Microsoft.AspNetCore.Mvc.BeforeAction")
//            {
//                ProcessBeforeAction(kv.Value);
//            }

//            if (kv.Key == "Microsoft.AspNetCore.Hosting.HttpRequestIn.Stop")
//            {
//                ProcessStopEvent(kv.Value);
//            }

//            if (kv.Key == "Microsoft.AspNetCore.Hosting.UnhandledException")
//            {
//                ProcessUnhandledException(kv.Value);
//            }
//        }

//        /// <summary>
//        /// process an unhandled exception
//        /// </summary>
//        /// <param name="value"></param>
//        private void ProcessUnhandledException(object value)
//        {
//            if (value.TryGetPropertyValue("exception", out Exception exception))
//            {
//                GlobalTracer.Instance.ActiveSpan.SetException(exception);
//            }
//        }

//        /// <summary>
//        /// process request finished
//        /// </summary>
//        /// <param name="value"></param>
//        private void ProcessStopEvent(object value)
//        {
//            if (value.TryGetPropertyValue("HttpContext", out HttpContext httpContext))
//            {
//                GlobalTracer.Instance.ActiveSpan
//                    .SetTag(Tags.HttpStatus, httpContext.Response.StatusCode);

//                GlobalTracer.Instance.ActiveSpan.Finish();
//            }
//        }

//        /// <summary>
//        /// process before action => extract binding information
//        /// </summary>
//        /// <param name="value"></param>
//        private void ProcessBeforeAction(object value)
//        {
//            if (value.TryGetPropertyValue("actionDescriptor", out object actionDescriptor))
//            {
//                actionDescriptor.TryGetPropertyValue("ActionName", out string actionName);
//                actionDescriptor.TryGetPropertyValue("ControllerName", out string controllerName);

//                GlobalTracer.Instance.ActiveSpan
//                    .SetTag("ActionName", actionName)
//                    .SetTag("ControllerName", controllerName);
//            }
//        }

//        /// <summary>
//        /// process request started event
//        /// </summary>
//        /// <param name="value"></param>
//        private void ProcessStartEvent(object value)
//        {
//            if (value.TryGetPropertyValue("HttpContext", out HttpContext httpContext))
//            {
//                var extracted = GlobalTracer.Instance
//                        .Extract(BuiltinFormats.HttpHeaders, new RequestHeadersExtractAdapter(httpContext));

//                GlobalTracer.Instance
//                    .BuildSpan(_configuration.Name)
//                    //.WithTag(Constants.AspNetCoreTraceIdentifier, httpContext.TraceIdentifier)
//                    .AsChildOf(extracted)
//                    .StartActive();

//                httpContext.Response.Headers.Add(Constants.TraceIdentifier, GlobalTracer.Instance.ActiveSpan.Context.TraceId);
//            }
//        }

//        public override bool ShouldSubscribe(DiagnosticListener diagnosticListener)
//        {
//            return diagnosticListener.Name == "Microsoft.AspNetCore" && _configuration.Enabled;
//        }
//    }
//}
