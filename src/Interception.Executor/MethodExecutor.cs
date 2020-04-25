using Interception.Tracing.Extensions;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;

namespace Interception.Common
{
    public static class MethodExecutor
    {
        public static object ExecuteMethod(object obj,
            object[] param,
            int mdToken,
            long moduleVersionPtr,
            bool noMetrics = false,
            string metricName = "",
            IDictionary<string, string> additionalTags = null,
            Type[] genericTypeArguments = null)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr, genericTypeArguments);
            if (method != null)
            {
                object result = null;
                if (!noMetrics)
                {
                    using (var scope = GlobalTracer.Instance.BuildSpan(metricName ?? "function_call").AsChildOf(GlobalTracer.Instance.ActiveSpan).StartActive())
                    {
                        if (additionalTags != null && additionalTags.Any())
                        {
                            foreach (var item in additionalTags)
                            {
                                scope.Span.SetTag(item.Key, item.Value);
                            }
                        }

                        try
                        {
                            result = ExecuteInternal(obj, param, method);
                        }
                        catch (Exception ex)
                        {
                            scope.Span.SetException(ex);
                            throw;
                        }
                    }
                }
                else
                {
                    result = ExecuteInternal(obj, param, method);
                }

                return result;
            }

            Console.WriteLine($"Not found call");

            return null;
        }

        private static object ExecuteInternal(object obj, object[] param, MethodBase method)
        {
            object result;
            Console.WriteLine($"Start calling {method.Name} ");
            result = method.Invoke(obj, param);

            Console.WriteLine($"Finish calling with result {result}");
            return result;
        }

        public static Task<T> ExecuteMethodAsync<T>(object obj,
            object[] param,
            int mdToken,
            long moduleVersionPtr,
            bool noMetrics = false,
            string metricName = "function_call",
            IDictionary<string, string> additionalTags = null,
            Type[] genericTypeArguments = null)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr, genericTypeArguments);
            if (method != null)
            {
                return ExecuteInternalAsync(() => {
                    var task = (Task<T>)method.Invoke(obj, param);
                    return task;
                }, method, metricName, additionalTags, noMetrics);
            }

            Console.WriteLine($"Not found call");

            return default;
        }

        private static async Task<T> ExecuteInternalAsync<T>(Func<Task<T>> action, MethodBase method, string metricName, IDictionary<string, string> additionalTags, bool noMetrics)
        {
            using (var scope = GlobalTracer.Instance.BuildSpan(metricName ?? "function_call").AsChildOf(GlobalTracer.Instance.ActiveSpan).StartActive())
            {
                if (additionalTags != null && additionalTags.Any())
                {
                    foreach (var item in additionalTags)
                    {
                        scope.Span.SetTag(item.Key, item.Value);
                    }
                }

                try
                {
                    return await action();
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }
    }
}
