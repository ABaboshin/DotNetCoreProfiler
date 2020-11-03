using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      try
      {
          TestM(1);
      }
      catch (System.Exception)
      {
      }
    }

    static int TestM(int i)
    {
      throw new Exception();
    }
  }
}
