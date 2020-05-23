using Interception.Attributes;
using System.Linq;

namespace Interception.Core
{
    /// <summary>
    /// base interceptor
    /// for attribute based interceptors
    /// </summary>
    public abstract class BaseAttributedInterceptor : BaseInterceptor
    {
        protected T GetCustomAttribute<T>() where T : MethodInterceptorAttribute
        {
            var attribute = (T)Method.GetCustomAttributes(typeof(T), false).First();

            return attribute;
        }
    }
}
