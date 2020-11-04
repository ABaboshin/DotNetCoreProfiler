using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      TestM(out var i);
    }

    static void TestM(out int i)
    {
      i = 3;
      Console.WriteLine(i);
    }
  }
}
