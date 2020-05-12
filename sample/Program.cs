using Interception.Attributes;
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using System;

namespace SampleApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Wrapped();

            CreateWebHostBuilder(args).Build().Run();
        }

        public static void Wrapped()
        {
            NewMethod();
        }

        private static void NewMethod()
        {
            int i = 1;
            object o = "test";
            int j = 4;
            object p = new { d = 3 };
            int k = -1;
            object q = null;
            TestM(ref o, ref i, out object oo, out int oi, out Program op, new Program(), out bool b, out UInt32 ou);
            //TestM(i, o, ref j, ref p, /*out*/ /*int*/ k, /*out*/ /*object*/ q);
        }

        public object GetObj()
        {
            return 111;
        }

        [Monitor]
        static void TestM(
            ref object q,
            ref int i,
            out object oo,
            out int oi,
            out Program op,
            object o,
            out bool b,
            out UInt32 ui
            //int i, object o, ref int j, ref object p, /*out*/ int k, /*out*/ object q
            )
        {
            var p = new Program();

            oo = p.GetObj();
            oi = (int)p.GetObj();
            op = p;// (Program)p.GetObj();
            b = true;
            ui = 1;

            Console.WriteLine($"TestM {q} {i}");

            //k = default;
            //q = null;

            //var data = new List<object>();
            //data.Add(i);
            //data.Add(o);
            //data.Add(j);
            //data.Add(p);
            //data.Add(k);
            //data.Add(q);

            //var method = typeof(Program).GetMethods(System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic).Where(m => m.Name == "TestM2").FirstOrDefault();
            //object[] parameters = data.ToArray();
            //method.Invoke(null, parameters);

            //j = (int)parameters[2];
            //p = parameters[3];
            //k = (int)parameters[4];
            //q = parameters[5];
        }

        static void TestM2(int i, object o, ref int j, ref object p, out int k, out object q)
        {
            j = 3;
            p = new { x = 1 };
            k = 5;
            q = new { y = 2 };
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
