using System;

namespace Interception.Attributes
{
    /// <summary>
    /// a loader attribute
    /// to force load interceptros
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class LoaderAttribute : Attribute
    {
    }

        /// <summary>
    /// a default attribute
    /// to create a default(T)
    /// </summary>
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = true, Inherited = false)]
    public class DefaultInitializerAttribute : Attribute
    {
    }

}
