using Interception.Metrics;
using System;
using System.Reflection;

namespace Interception.Common
{
    public static class MethodExecutor
    {
        public static object ExecuteMethod(object obj, object[] param, int mdToken, long moduleVersionPtr, bool noMetrics = false)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr);
            if (method != null)
            {
                object result = null;
                if (!noMetrics)
                {
                    MetricsSender.Histogram(() =>
                    {
                        result = ExecuteInternal(obj, param, method);
                    }, method);
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
