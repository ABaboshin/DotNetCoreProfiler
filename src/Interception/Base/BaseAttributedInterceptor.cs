using Interception.Attributes;
using System.Linq;

namespace Interception.Base
{
    /// <summary>
    /// base interceptor
    /// for attribute based interceptors
    /// </summary>
    public abstract class BaseAttributedInterceptor : BaseInterceptor
    {
        protected T GetCustomAttribute<T>() where T : MethodInterceptorAttribute
        {
            var method = FindMethod();
            var attribute = (T)method.GetCustomAttributes(typeof(T), false).First();

            return attribute;
        }
    }
}
