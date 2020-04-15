using Interception.Metrics;
using Interception.Metrics.Extensions;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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
            IEnumerable<string> additionalTags = null,
            Type[] genericTypeArguments = null)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr, genericTypeArguments);
            if (method != null)
            {
                object result = null;
                if (!noMetrics)
                {
                    MetricsSender.Histogram(() =>
                    {
                        result = ExecuteInternal(obj, param, method);
                    }, method, metricName, additionalTags);
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
            IEnumerable<string> additionalTags = null,
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

        private static async Task<T> ExecuteInternalAsync<T>(Func<Task<T>> action, MethodBase method, string metricName, IEnumerable<string> additionalTags, bool noMetrics)
        {
            var sw = new Stopwatch();
            sw.Start();
            Exception exception = null;
            try
            {
                return await action();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Catched exception");
                exception = ex;
                throw;
            }
            finally
            {
                if (!noMetrics)
                {
                    sw.Stop();

                    var tags = new List<string> { $"success:{exception is null}", $"name:{method.DeclaringType.Name}.{method.Name}" };
                    if (exception != null)
                    {
                        tags.AddRange(exception.GetTags());
                    }

                    if (additionalTags != null)
                    {
                        tags.AddRange(additionalTags);
                    }

                    MetricsSender.Histogram(metricName, (double)sw.ElapsedMilliseconds, tags);
                }
            }
        }

        public static Task ExecuteMethodAsync(object obj,
            object[] param,
            int mdToken,
            long moduleVersionPtr,
            bool noMetrics = false,
            string metricName = "function_call",
            IEnumerable<string> additionalTags = null,
            Type[] genericTypeArguments = null)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr, genericTypeArguments);
            if (method != null)
            {
                return ExecuteInternalAsync(() => {
                    var task = (Task)method.Invoke(obj, param);
                    return task;
                }, method, metricName, additionalTags, noMetrics);
            }

            Console.WriteLine($"Not found call");

            return default;
        }

        public static async Task ExecuteInternalAsync(Func<Task> action, MethodBase method, string metricName, IEnumerable<string> additionalTags, bool noMetrics)
        {
            var sw = new Stopwatch();
            sw.Start();
            Exception exception = null;
            try
            {
                await action();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Catched exception");
                exception = ex;
                throw;
            }
            finally
            {
                if (!noMetrics)
                {
                    sw.Stop();

                    var tags = new List<string> { $"success:{exception is null}" };
                    if (method != null)
                    {
                        tags.Add($"name:{method.DeclaringType.Name}.{method.Name}");
                    }
                    if (exception != null)
                    {
                        tags.AddRange(exception.GetTags());
                    }

                    if (additionalTags != null)
                    {
                        tags.AddRange(additionalTags);
                    }

                    MetricsSender.Histogram(metricName, (double)sw.ElapsedMilliseconds, tags);
                }
            }
        }
    }
}
