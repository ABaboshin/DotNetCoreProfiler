using System;

namespace Interception.Attributes.Validation
{
    /// <summary>
    /// check if a parameter is not null
    /// </summary>
    [AttributeUsage(AttributeTargets.Parameter, AllowMultiple = false, Inherited = false)]
    public class NotNullAttribute : ParameterValidationAttribute
    {
        public override void Validate(object value)
        {
            if (value is null)
            {
                throw new ArgumentNullException();
            }
        }
    }
}
