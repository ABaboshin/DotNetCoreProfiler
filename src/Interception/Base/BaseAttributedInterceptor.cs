using Interception.Attributes;
using System.Linq;

namespace Interception.Base
{
    public abstract class BaseAttributedInterceptor : BaseInterceptor
    {
        protected T GetCustomAttribute<T>() where T : IMethodInterceptorAttribute
        {
            var method = FindMethod();
            var attribute = (T)method.GetCustomAttributes(typeof(T), false).First();

            return attribute;
        }
    }
}
