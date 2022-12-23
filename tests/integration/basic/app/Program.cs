using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      // // interceptor.C1.Before();
      // // Exception ex = null;
      // try
      // {
      //   try
      //   {
      //       interceptor.C1.Before();
      //   }
      //   catch (System.Exception)
      //   {
      //   }

          // TestM();

          var c = new C1();
          c.M3();
          c.M4("test", new C1());

          C1.M1();
          C1.M11();
          C1.M2(3);
      // }
      // catch (System.Exception)
      // {
      //   // ex = e;
      //   throw;
      // }
      // finally {
      //   try
      //   {
      //       interceptor.C1.After();
      //   }
      //   catch (System.Exception)
      //   {
      //   }
      // }

    }

    // static void TestM()
    // {
    //   Console.WriteLine("TestM");
    // }
  }

  public class C1 {
    public static void M1() {}
    public static int M11() {return 1;}
    public static void M2(int i) { }

    public void M3() { }
    public void M4(string s, object o) { }
  }
}
