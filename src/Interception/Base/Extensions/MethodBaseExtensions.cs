using System.Reflection;
using System.Threading.Tasks;

namespace Interception.Base.Extensions
{
    public static class MethodBaseExtensions
    {
        public static bool IsReturnTypeTask(this MethodBase method)
        {
            var methodInfo = (MethodInfo)method;
            var returnType = methodInfo.ReturnType;
            return returnType.IsReturnTypeTask();
        }

        public static bool IsReturnTypeTaskWithResult(this MethodBase method)
        {
            var methodInfo = (MethodInfo)method;
            var returnType = methodInfo.ReturnType;
            return returnType.IsReturnTypeTask() && returnType != typeof(Task);
        }
    }
}
