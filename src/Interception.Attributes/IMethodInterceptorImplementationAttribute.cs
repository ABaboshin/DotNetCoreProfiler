using System;

namespace Interception.Attributes
{
    /// <summary>
    /// base interceptor attribute
    /// </summary>
    public interface IMethodInterceptorImplementationAttribute
    {
        Type MethodInterceptorAttribute { get; set; }
    }
}
