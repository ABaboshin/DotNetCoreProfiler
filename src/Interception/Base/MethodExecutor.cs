using System;
using System.Reflection;
using System.Threading.Tasks;

namespace Interception.Base
{
    public class MethodExecutor : IMethodExecutor
    {
        public object ExecuteSync(MethodBase method, object obj, object[] param)
        {
            return method.Invoke(obj, param);
        }

        public async Task<T> ExecuteAsync<T>(MethodBase method, object obj, object[] param)
        {
            var result = (Task<T>)method.Invoke(obj, param);
            return await result;
        }

        public async Task ExecuteAsync(MethodBase method, object obj, object[] param)
        {
            var result = (Task)method.Invoke(obj, param);
            await result;
        }

        public bool IsReturnTypeTask(MethodBase method)
        {
            var methodInfo = (MethodInfo)method;
            var returnType = methodInfo.ReturnType;
            return IsReturnTypeTask(returnType);
        }

        private bool IsReturnTypeTask(Type returnType)
        {
            if (returnType == typeof(Task))
            {
                return true;
            }

            if (returnType.BaseType != null)
            {
                return IsReturnTypeTask(returnType.BaseType);
            }

            return false;
        }
    }
}
