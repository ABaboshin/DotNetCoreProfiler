using System;

namespace app
{
    class Program
    {
        static void Main(string[] args)
        {

            var x = new X1();
            x.MX<int>(3);
            x.Test(3);

            //var c = new C1();
            //c.M3();
            //try
            //{
            //    c.M4("test", new C1());
            //}
            //catch (Exception e)
            //{
            //    Console.WriteLine($"Exception {e}");
            //}

            //C1.M1();
            //C1.M11();
            //var teststr = "i";
            //C1.M2(teststr);
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

    public interface I1<TClassSpec>
    {
        TClassSpec MX<TMethodSpec>(TMethodSpec p);
    }

    public abstract class X1<TClassSpec> : I1<TClassSpec>
    {
        public abstract TClassSpec MX<TMethodSpec>(TMethodSpec p);

        public void Test(int i) {
            Console.WriteLine($"Test void {i}");
        }
    }

    public class X1 : X1<string>
    {
        public override string MX<TMethodSpec>(TMethodSpec p)
        {
            Console.WriteLine("MX!!!");
            return p.ToString();
        }
    }
}
