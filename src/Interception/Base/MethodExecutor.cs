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

        public Task ExecuteAsyncWithResult(MethodBase method, object obj, object[] param)
        {
            return (Task)method.Invoke(obj, param);
        }

        public async Task ExecuteAsync(MethodBase method, object obj, object[] param)
        {
            var result = (Task)method.Invoke(obj, param);
            await result.ConfigureAwait(false);
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

        public bool IsReturnTypeTaskWithResult(MethodBase method)
        {
            var methodInfo = (MethodInfo)method;
            var returnType = methodInfo.ReturnType;
            return IsReturnTypeTask(returnType) && returnType != typeof(Task);
        }
    }
}
