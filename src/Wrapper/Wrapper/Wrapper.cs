using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;

namespace Wrapper
{
    public interface IWrapper
    {
        void Start();
        void Finish(object returnValue, object exception);
    }

    public class Wrapper : IWrapper
    {
        public Wrapper()
        {
            Console.Write("Wrapper.ctor");
        }

        public void Test(/*int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8*/params object[] vs)
        {
            foreach (var v in vs)
            {
                Console.WriteLine(v);
            }
            //var i = p1;
            //i = p1;
            //i = p2;
            //i = p3;
            //i = p4;
            //i = p5;
            //i = p6;
            //i = p7;
            //i = p8;
        }

        public static void Test(uint functionToken)
        {
            Console.WriteLine($"Wrapper.Test 42 functionToken {functionToken}");
        }

        //public static void Test()
        //{
        //    Console.WriteLine("Wrapper.Test 42 _0_");
        //}

        public static void TestVoid(object[] param, string typeName, string funcitonName, int functionToken)
        {
            //Console.WriteLine($"Wrapper.Test 42 {param.Length} {functionToken} {typeName} {assemblyName}");

            Console.WriteLine($"Invoke params {string.Join(", ", param.Select(p => p.GetType().Name))} {typeName}.{funcitonName}");

            foreach (var item in param)
            {
                Console.WriteLine($" Invoke param value " + item.ToString());
            }

            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                foreach (var type in assembly.GetTypes())
                {
                    if (type.FullName == typeName)
                    {
                        foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Instance))
                        {
                            if (method.MetadataToken == functionToken)
                            {
                                Console.WriteLine($"Found isStatic {method.IsStatic}");
                                Console.WriteLine("call");
                                method.Invoke(null, param);
                                Console.WriteLine("call done");
                            }
                        }
                    }
                }
            }
        }

        public static object TestRet(object[] param, string typeName, string functionName, int functionToken)
        {
            //Console.WriteLine($"Wrapper.Test 42 {param.Length} {functionToken} {typeName} {assemblyName}");

            Console.WriteLine($"Invoke params {string.Join(", ", param.Select(p => p.GetType().Name))} {typeName}.{functionName}");

            foreach (var item in param)
            {
                Console.WriteLine($" Invoke param value " + item.ToString());
            }

            return new object();

            //foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            //{
            //    foreach (var type in assembly.GetTypes())
            //    {
            //        if (type.FullName == typeName)
            //        {
            //            foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Instance))
            //            {
            //                if (method.MetadataToken == functionToken)
            //                {
            //                    Console.WriteLine($"Found isStatic {method.IsStatic}");
            //                    Console.WriteLine("call");
            //                    method.Invoke(null, param);
            //                    Console.WriteLine("call done");
            //                }
            //            }
            //        }
            //    }
            //}
        }

        private readonly Stopwatch _stopwatch = new Stopwatch();

        //public Wrapper()
        //{
        //    Console.WriteLine("Wrapper.ctor");
        //}

        public void Finish(object returnValue, object exception)
        {
            _stopwatch.Stop();
            Console.WriteLine($"Wrapper.Finish {_stopwatch.ElapsedMilliseconds} {returnValue} {exception}");
            //Console.WriteLine($"Wrapper.Finish");
        }

        public void Start()
        {
            _stopwatch.Start();
            Console.WriteLine("Wrapper.Start");
        }
    }
}
