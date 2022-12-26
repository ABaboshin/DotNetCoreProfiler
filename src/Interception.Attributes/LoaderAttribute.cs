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
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class DefaultInitializerAttribute : Attribute
    {
    }

    /// <summary>
    /// an exception logger attribute
    /// to log before/after exception
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
    public class ExceptionLoggerAttribute : Attribute
    {
    }

}
