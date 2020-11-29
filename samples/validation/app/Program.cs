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

    [Validation]
    static void TestM([CustomValidation] int i)
    {
    }
  }

  public class CustomValidationAttribute : ParameterValidationAttribute
  {
    public override void Validate(object value)
    {
      var i = (int)value;
      if (i > 1 && i < 10)
      {
        throw new ArgumentOutOfRangeException();
      }
    }
  }
}
