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
        private int i;
        public void Test(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9)
        {
            i = p1;
            Console.WriteLine($"Wrapper.Test 42 functionToken");
        }

        public static void Test(object[] param, int functionToken, string typeName, string assemblyName)
        {
            //Console.WriteLine($"Wrapper.Test 42 {param.Length} {functionToken} {typeName} {assemblyName}");

            Console.WriteLine($"Invoke params {string.Join(", ", param.Select(p => p.GetType().Name))}");

            foreach (var item in param)
            {
                Console.WriteLine($" Invoke param value " + item.ToString());
            }

            try
            {
                var type = Type.GetType($"{typeName}, {assemblyName}");
                //Console.WriteLine($"Found {typeName} {type}");
                foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Instance))
                {
                    if (method.MetadataToken == functionToken)
                    {
                        //Console.WriteLine($"Wrapped {method.Name} {method.IsVirtual}");

                        Console.WriteLine($"Invoke {method.Name}");
                        //method.Invoke(null, param);

                        //var p = method.GetParameters().Select(p => p.ParameterType)
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        public void Finish(object returnValue, object exception)
        {
        }

        public void Start()
        {
        }
    }
}
