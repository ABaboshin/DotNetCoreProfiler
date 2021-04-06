using Interception.Core.Extensions;
using Interception.Observers.Configuration;
using Interception.Tracing.Extensions;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Http;
using System.Threading.Tasks;

namespace Interception.Observers
{
    public class HttpHandlerDiagnostrics : BaseObserver
    {
        private static readonly string PropertiesKey = "X-Interception";

        private readonly HttpHandlerConfiguration _configuration;

        public HttpHandlerDiagnostrics(HttpHandlerConfiguration configuration)
        {
            _configuration = configuration;
        }

        public override void OnCompleted()
        {
        }

        public override void OnError(Exception error)
        {
        }

        public override void OnNext(KeyValuePair<string, object> value)
        {
            if (value.Key == "System.Net.Http.HttpRequestOut.Start")
            {
                ProcessStartEvent(value.Value);
            }

            if (value.Key == "System.Net.Http.Exception")
            {
                ProcessException(value);
            }

            if (value.Key == "System.Net.Http.HttpRequestOut.Stop")
            {
                ProcessStopEvent(value.Value);
            }
        }

        private void ProcessStopEvent(object value)
        {
            if (value.TryGetPropertyValue("Request", out HttpRequestMessage request))
            {
                if (request.Properties.TryGetValue(PropertiesKey, out object obj) && obj is ISpan span)
                {
                    if (value.TryGetPropertyValue("Response", out HttpResponseMessage response) && response != null)
                    {
                        span.SetTag(Tags.HttpStatus, (int)response.StatusCode);
                    }

                    if (value.TryGetPropertyValue("RequestTaskStatus", out TaskStatus requestTaskStatus))
                    {
                        if (requestTaskStatus == TaskStatus.Canceled || requestTaskStatus == TaskStatus.Faulted)
                        {
                            span.SetTag(Tags.Error, true);
                        }
                    }

                    span.Finish();

                    request.Properties.Remove(PropertiesKey);
                }
            }
        }

        private void ProcessException(KeyValuePair<string, object> value)
        {
            if (value.TryGetPropertyValue("Request", out HttpRequestMessage request))
            {
                if (request.Properties.TryGetValue(PropertiesKey, out object obj) && obj is ISpan span)
                {
                    if (value.TryGetPropertyValue("Exception", out Exception ex))
                    {
                        span
                            .SetException(ex);
                    }
                }
            }
        }

        private void ProcessStartEvent(object value)
        {
            if (value.TryGetPropertyValue("Request", out HttpRequestMessage request))
            {
                var span = GlobalTracer.Instance
                    .BuildSpan(_configuration.Name)
                    .WithTag(Tags.HttpUrl, request.RequestUri.ToString())
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient)
                    .AsChildOf(GlobalTracer.Instance.ActiveSpan)
                    .Start();

                GlobalTracer.Instance.Inject(span.Context, BuiltinFormats.HttpHeaders, new RequestHeadersInjectAdapter(request));

                request.Properties.Add(PropertiesKey, span);
            }
        }

        public override bool ShouldSubscribe(DiagnosticListener diagnosticListener)
        {
            return diagnosticListener.Name == "HttpHandlerDiagnosticListener" && _configuration.Enabled;
        }
    }
}
