using System;

namespace Interception.Attributes
{
    /// <summary>
    /// base interceptor attribute
    /// </summary>
    public interface IInterceptorAttribute
    {
        Type UserAttributeType { get; }
    }
}
