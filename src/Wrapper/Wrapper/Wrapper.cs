using System;
using System.Diagnostics;

namespace Wrapper
{
    public interface IWrapper
    {
        void Start();
        void Finish(object returnValue, object exception);
    }

    public class Wrapper : IWrapper
    {
        public static void Test(uint functionToken)
        {
            Console.WriteLine($"Wrapper.Test 42 functionToken {functionToken}");
        }

        public static void Test()
        {
            Console.WriteLine("Wrapper.Test 42 _0_");
        }

        public static void Test(object[] param, int functionToken, string typeName, string assemblyName)
        {
            Console.WriteLine($"Wrapper.Test 42 {param.Length} {functionToken} {typeName} {assemblyName}");

            try
            {
                var type = Type.GetType($"{typeName}, {assemblyName}");
                Console.WriteLine($"Found {typeName} {type}");
                foreach (var method in type.GetMethods())
                {
                    if (method.MetadataToken == functionToken)
                    {
                        Console.WriteLine($"Wrapped {method.Name}");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        //private readonly Stopwatch _stopwatch = new Stopwatch();

        //public Wrapper()
        //{
        //    Console.WriteLine("Wrapper.ctor");
        //}

        public void Finish(object returnValue, object exception)
        {
            //_stopwatch.Stop();
            //Console.WriteLine($"Wrapper.Finish {_stopwatch.ElapsedMilliseconds} {returnValue} {exception}");
        }

        public void Start()
        {
            //_stopwatch.Start();
            //Console.WriteLine("Wrapper.Start");
        }
    }
}
