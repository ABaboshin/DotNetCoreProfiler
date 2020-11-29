using System;
using Interception.Attributes.Validation;

namespace app
{
  class Program
  {
    static void Main(string[] args)
    {
      TestM(3);
    }

    [SampleAttributes.MySampleAttribute(P1 = 6000, P2 = new[] { "an", "array" })]
    static void TestM(int i)
    {
    }
  }
}
