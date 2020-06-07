using System;

namespace Interception.Attributes.Validation
{
    /// <summary>
    /// parameter validation attribute
    /// </summary>
    public abstract class ParameterValidationAttribute : Attribute
    {
        /// <summary>
        /// validate parameter value
        /// throw an exception if validation failed
        /// </summary>
        /// <param name="value"></param>
        public abstract void Validate(object value);
    }
}
