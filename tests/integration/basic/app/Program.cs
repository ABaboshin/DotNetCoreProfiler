using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
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
          c.M4("test", new C1());

          C1.M1();
          C1.M11();
          C1.M2(3);
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
    }
  }

  public class C1 {
    public static void M1() {}
    public static int M11() {return 1;}
    public static void M2(int i) { }

    public void M3() { }
    public void M4(string s, object o) { }
  }
}
