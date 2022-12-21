using Interception.Attributes;
using Interception.Attributes.Validation;
using System.Reflection;

namespace Interception.Core
{
    [MethodInterceptorImplementation(typeof(ValidationAttribute))]
    public class ValidationInterceptor : BaseInterceptor
    {
        public override int Priority => 100;

        public override void ExecuteBefore()
        {
            foreach (var p in Method.GetParameters())
            {
                var validationAttributes = p.GetCustomAttributes<ParameterValidationAttribute>();

                foreach (var validationAttribute in validationAttributes)
                {
                    validationAttribute.Validate(GetParameter(p.Position));
                }
            }
        }
    }

    public class Loader {
      static Loader () {
        System.Console.WriteLine("Loader");
      }
    }
}
