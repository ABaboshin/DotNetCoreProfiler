using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Wrapper.Common;

namespace Wrapper
{
    public static class ValuesController
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "SampleApp", TargetMethodName = "Test", TargetTypeName = "SampleApp.Controllers.ValuesController")]
        public static object TestReplace(object controller, object i, object g, object o, int mdToken, long moduleVersionPtr)
        {
            //Console.WriteLine($"Call TestReplace moduleVersionPtr {moduleVersionPtr}");
            //Console.WriteLine($"Call TestReplace mdtoken {mdToken}");
            ////Console.WriteLine($"Call TestReplace o {o}");
            //Console.WriteLine($"Call TestReplace g {g}");
            //Console.WriteLine($"Call TestReplace i {i}");
            //Console.WriteLine($"Call TestReplace {controller}");






            Console.WriteLine($"Call TestReplace {controller} {i} {g} {o} {mdToken} {moduleVersionPtr}");
            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr);
            if (method != null)
            {
                object result = null;
                Metrics.Histogram(() => {
                    Console.WriteLine($"Start calling {method.Name} ");
                    result = method.Invoke(controller, new object[] { i, g, o });

                    Console.WriteLine($"Finish calling with result {result}");
                }, method);

                return result;
            }

            Console.WriteLine($"Not found call");

            return Guid.Empty;
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "SampleApp", TargetMethodName = "TestStatic", TargetTypeName = "SampleApp.Controllers.ValuesController")]
        public static object TestStaticReplace(object controller, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"Call TestStaticReplace {controller} {mdToken} {moduleVersionPtr}");
            var method = MethodFinder.FindMethod(mdToken, moduleVersionPtr);
            if (method != null)
            {
                object result = null;
                Metrics.Histogram(() => {
                    Console.WriteLine($"Start calling {method.Name} ");
                    result = method.Invoke(null, new object[] { controller });

                    Console.WriteLine($"Finish calling with result {result}");
                }, method);

                return result;
            }

            Console.WriteLine($"Not found call");

            return Guid.Empty;
        }
    }
}
