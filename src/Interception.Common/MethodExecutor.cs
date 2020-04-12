using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Common
{
    public static class MethodExecutor
    {
        public static object ExecuteMethod(object obj, object[] param, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"Call MethodExecutor.ExecuteMethod {mdToken} {moduleVersionPtr}");

            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr);
            if (method != null)
            {
                object result = null;
                Metrics.Histogram(() => {
                    Console.WriteLine($"Start calling {method.Name} ");
                    result = method.Invoke(obj, param);

                    Console.WriteLine($"Finish calling with result {result}");
                }, method);

                return result;
            }

            Console.WriteLine($"Not found call");

            return null;
        }
    }
}
