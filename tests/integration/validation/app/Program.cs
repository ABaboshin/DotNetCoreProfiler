using System;
using Interception.Attributes.Validation;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      try
      {
        TestM(-2);
      }
      catch (System.ArgumentOutOfRangeException e)
      {
        Console.WriteLine("Validation Exception");
      }
    }

    [Validation]
    static void TestM([GreatThenZero] int i)
    {
    }
  }
}
