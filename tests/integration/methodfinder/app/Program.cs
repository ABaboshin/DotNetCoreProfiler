using System;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      new TC<string>().TestM(1);
    }
  }

  public class TC<T>
  {
    public void TestM(int i)
    {
      Console.WriteLine("oops");
    }
  }
}
