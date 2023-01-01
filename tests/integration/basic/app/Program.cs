using System;
using System.Threading.Tasks;

namespace app
{
    class Program
    {
        static async Task Main(string[] args)
        {
            //var x = new X1();
            //x.MX<int>(3);
            //await x.Test(3);

            C1.M1();
            //C1.M11();
            //C1.M2("teststr");

            var c1 = new C1();
            //c1.M3();
            try
            {
                c1.M4(123, "strparam");
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }
    }

    public class C1
    {
        public static void M1() { }
        public static C1 M11()
        {
            return new C1();
        }
        public static string M2(string i)
        {
            Console.WriteLine($"M2 {i}");
            return i;
        }

        public void M3() { }
        public int M4<T1, T2>(T1 s, T2 o)
        {
            return 1;
            //Console.WriteLine($"M4 {s} {o}");
            //throw new Exception();
        }
    }

    public interface I1<TClassSpec>
    {
        TClassSpec MX<TMethodSpec>(TMethodSpec p);
    }

    public abstract class X1<TClassSpec> : I1<TClassSpec>
    {
        public abstract TClassSpec MX<TMethodSpec>(TMethodSpec p);

        public Task<int> Test(int i)
        {
            Console.WriteLine($"Test void {i}");
            return XXX();
        }

        private async Task<int> XXX()
        {
            await Task.Delay(1000);
            Console.WriteLine($"Test XXX");
            return await Task.FromResult(0);
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
