using System.Reflection;
using System.Threading.Tasks;

namespace Interception.Base
{
    public interface IMethodExecutor
    {
        Task ExecuteAsync(MethodBase method, object obj, object[] param);
        Task<T> ExecuteAsync<T>(MethodBase method, object obj, object[] param);
        object ExecuteSync(MethodBase method, object obj, object[] param);
        bool IsReturnTypeTask(MethodBase method);
    }
}