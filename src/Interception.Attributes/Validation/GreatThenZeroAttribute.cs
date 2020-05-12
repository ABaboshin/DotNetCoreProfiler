using System;

namespace Interception.Attributes.Validation
{
    public class GreatThenZeroAttribute : ParameterValidationAttribute
    {
        public override void Validate(object value)
        {
            if ((int)value < 1)
            {
                throw new ArgumentOutOfRangeException();
            }
        }
    }
}
