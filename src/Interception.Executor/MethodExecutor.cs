using Interception.Metrics;
using System;
using System.Collections.Generic;
using System.Reflection;

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
    }
}
