using Interception.Common;
using Interception.Common.Extensions;
using System;
using System.Collections.Generic;
using System.Text;

namespace Interception
{
    public class AspNetCoreInterception : IDisposable
    {
        private static readonly string HttpContextKey = "ClrProfiler";

        private readonly object _httpContext;
        private readonly DateTime _start = DateTime.UtcNow;
        private readonly object _traceIdentifier;
        private readonly object _controllerName;
        private readonly object _actionName;
        private object _statusCode;
        private Exception _exception;

        public AspNetCoreInterception(object httpContext, object actionDescriptor)
        {
            _httpContext = httpContext;

            httpContext.TryGetPropertyValue("TraceIdentifier", out _traceIdentifier);

            actionDescriptor.TryGetPropertyValue("ControllerName", out _controllerName);

            actionDescriptor.TryGetPropertyValue("ActionName", out _actionName);
        }

        //[Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Mvc.Core", TargetMethodName = "BeforeAction", TargetTypeName = "Microsoft.AspNetCore.Mvc.Internal.MvcCoreDiagnosticSourceExtensions", TargetMethodParametersCount = 4)]
        public static void BeforeAction(object diagnosticSource, object actionDescriptor, object httpContext, object routeData, int mdToken, long moduleVersionPtr)
        {
            var integration = new AspNetCoreInterception(httpContext, actionDescriptor);
            if (httpContext.TryGetPropertyValue("Items", out IDictionary<object, object> contextItems))
            {
                Console.WriteLine("Create Integration");
                contextItems[HttpContextKey] = integration;
            }
            else
            {
                Console.WriteLine("Skip Create Integration");
            }

            try
            {
                MethodExecutor.ExecuteMethod(null, new object[] { diagnosticSource, actionDescriptor, httpContext, routeData }, mdToken, moduleVersionPtr, true);
            }
            catch (Exception ex)
            {
                integration._exception = ex;
                throw;
            }
        }

        //[Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Mvc.Core", TargetMethodName = "AfterAction", TargetTypeName = "Microsoft.AspNetCore.Mvc.Internal.MvcCoreDiagnosticSourceExtensions", TargetMethodParametersCount = 4)]
        public static void AfterAction(object diagnosticSource, object actionDescriptor, object httpContext, object routeData, int mdToken, long moduleVersionPtr)
        {
            AspNetCoreInterception integration = default;
            if (httpContext.TryGetPropertyValue("Items", out IDictionary<object, object> contextItems))
            {
                integration = (AspNetCoreInterception)contextItems[HttpContextKey];
            }

            try
            {
                MethodExecutor.ExecuteMethod(null, new object[] { diagnosticSource, actionDescriptor, httpContext, routeData }, mdToken, moduleVersionPtr, true);
            }
            catch (Exception ex)
            {
                integration._exception = ex;
                throw;
            }
        }

        //[Intercept(CallerAssembly = "", TargetAssemblyName = "Microsoft.AspNetCore.Mvc.Core", TargetMethodName = "Rethrow", TargetTypeName = "Microsoft.AspNetCore.Mvc.Internal.ResourceInvoker", TargetMethodParametersCount = 4)]
        public static void Rethrow(object httpContext, int mdToken, long moduleVersionPtr)
        {
            AspNetCoreInterception integration = default;
            if (httpContext.TryGetPropertyValue("Items", out IDictionary<object, object> contextItems))
            {
                integration = (AspNetCoreInterception)contextItems[HttpContextKey];
            }

            try
            {
                MethodExecutor.ExecuteMethod(null, new object[] { httpContext }, mdToken, moduleVersionPtr, true);
            }
            catch (Exception ex)
            {
                integration._exception = ex;
                throw;
            }
        }

        public void Dispose()
        {
            Console.WriteLine("Dispose Integration");

            if (_httpContext != null &&
                    _httpContext.TryGetPropertyValue("Response", out object response) &&
                    response.TryGetPropertyValue("StatusCode", out object statusCode))
            {
                _statusCode = statusCode;
            }

            var tags = new List<string> {
                $"traceIdentifier:{_traceIdentifier}",
                $"controllerName:{_controllerName}",
                $"actionName:{_actionName}",
                $"statusCode:{_statusCode}",
            };

            if (_exception != null)
            {
                tags.AddRange(_exception.GetTags());
            }

            Metrics.Histogram("http_call", (double)(DateTime.UtcNow - _start).TotalMilliseconds, tags.ToArray());
        }
    }
}
