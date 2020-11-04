using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      int i = 1;
      TestM(ref i);
    }

    static void TestM(ref int i)
    {
      i = 3;
      Console.WriteLine(i);
    }
  }
}
