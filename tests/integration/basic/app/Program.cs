using System;

namespace app
{
    class Program
    {
        static int Main(string[] args)
        {
            //var test = typeof(interceptor.DefaultInitializer);
            //var m1 = test.GetMethod("GetDefault");
            //var mb1 = test.Module.ResolveMethod(m1.MetadataToken);

            //test.Module.ResolveMethod(test.DEcla)
            //interceptor.DefaultInitializer.GetDefault<int>();
            // // interceptor.C1.Before();
            //       Exception ex = null;
            //       int i = 0;
            //       try
            //       {
            //         try
            //         {
            //             interceptor.C1.Before();
            //         }
            //         catch (System.Exception)
            //         {
            //         }

            // // throw new Exception("test");
            // i = 3;

            var c = new C1();
            c.M3();
            try
            {
                c.M4("test", new C1());
            }
            catch (Exception e)
            {
                Console.WriteLine($"Exception {e}");
            }

            C1.M1();
            C1.M11();
            var teststr = "i";
            C1.M2(teststr);

            // }
            // catch (System.Exception e)
            // {
            //   ex = e;
            // }
            // finally {
            //   try
            //   {
            //       interceptor.C1.After();
            //   }
            //   catch (System.Exception)
            //   {
            //   }
            //   if (ex != null) throw ex;
            // }
            // return i;
            //System.Diagnostics.Debugger.Break();
            return 0;
        }
    }

    public class C1
    {
        public static void M1() { }
        public static C1 M11()
        {
            return new C1();
        }
        public static string M2(string i) {
            Console.WriteLine($"M2 {i}");
            return i;
        }

        public void M3() { }
        public int M4<T1, T2>(T1 s, T2 o) {
            Console.WriteLine($"M4 {s} {o}");
            throw new Exception();
        }
    }
}
