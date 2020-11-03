using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      TestM(out var i);
    }

    static void TestM(out object i)
    {
      i = new object();
    }
  }
}
