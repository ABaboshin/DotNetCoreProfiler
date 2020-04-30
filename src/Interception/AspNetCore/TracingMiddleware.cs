using Interception.Observers.Configuration;
using Interception.Tracing;
using Interception.Tracing.Extensions;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Routing;
using Microsoft.Extensions.Options;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Threading.Tasks;

namespace Interception.AspNetCore
{
    public class TracingMiddleware
    {
        private readonly RequestDelegate _next;
        private readonly AspNetCoreConfiguration _configuration;

        public TracingMiddleware(RequestDelegate next, IOptions<AspNetCoreConfiguration> options)
        {
            _next = next;
            _configuration = options.Value;
        }

        public async Task Invoke(HttpContext context)
        {
            var extracted = GlobalTracer.Instance
                    .Extract(BuiltinFormats.HttpHeaders, new RequestHeadersExtractAdapter(context));

            GlobalTracer.Instance
                .BuildSpan(_configuration.Name)
                .AsChildOf(extracted)
                .StartActive();

            var traceId = GlobalTracer.Instance.ActiveSpan.Context.TraceId;

            context.Response.OnStarting(() =>
            {
                if (!context.Response.Headers.ContainsKey(Constants.TraceIdentifier))
                {
                    context.Response.Headers.Add(Constants.TraceIdentifier, traceId);
                }

                return Task.CompletedTask;
            });

            try
            {
                await _next(context);
            }
            catch (Exception ex)
            {
                GlobalTracer.Instance.ActiveSpan.SetException(ex);
                throw;
            }
            finally
            {
                GlobalTracer.Instance.ActiveSpan
                    .SetTag(Tags.HttpStatus, context.Response.StatusCode);

                var routeData = context.GetRouteData();

                var actionName = routeData?.Values["Action"] as string ?? string.Empty;
                var controllerName = routeData?.Values["Controller"] as string ?? string.Empty;

                GlobalTracer.Instance.ActiveSpan
                    .SetTag("ActionName", actionName)
                    .SetTag("ControllerName", controllerName);

                GlobalTracer.Instance.ActiveSpan.Finish();
            }
        }
    }
}
