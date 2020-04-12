using Interception.Common;
using System;
using System.Data;
using System.Threading;

namespace Interception
{
    public static class DbCommandInterception
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteReader", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 0)]
        public static object ExecuteReader(object command, int mdToken, long moduleVersionPtr)
        {
            return MethodExecutor.ExecuteMethod(command, new object[] { }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteReaderAsync", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 2)]
        public static object ExecuteReaderAsync(object command, int behavior, object cancellationTokenSource, int mdToken, long moduleVersionPtr)
        {
            var tokenSource = cancellationTokenSource as CancellationTokenSource;
            var cancellationToken = tokenSource?.Token ?? CancellationToken.None;

            return MethodExecutor.ExecuteMethod(command, new object[] { (CommandBehavior)behavior, cancellationToken }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteNonQuery", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 0)]
        public static object ExecuteNonQuery(object command, int mdToken, long moduleVersionPtr)
        {
            return MethodExecutor.ExecuteMethod(command, new object[] { }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteNonQueryAsync", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 1)]
        public static object ExecuteNonQueryAsync(object command, object cancellationTokenSource, int mdToken, long moduleVersionPtr)
        {
            var tokenSource = cancellationTokenSource as CancellationTokenSource;
            var cancellationToken = tokenSource?.Token ?? CancellationToken.None;

            return MethodExecutor.ExecuteMethod(command, new object[] { cancellationToken }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteScalar", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 0)]
        public static object ExecuteScalar(object command, int mdToken, long moduleVersionPtr)
        {
            return MethodExecutor.ExecuteMethod(command, new object[] { }, mdToken, moduleVersionPtr);
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "System.Data.Common", TargetMethodName = "ExecuteScalarAsync", TargetTypeName = "System.Data.Common.DbCommand", TargetMethodParametersCount = 1)]
        public static object ExecuteScalarAsync(object command, object cancellationTokenSource, int mdToken, long moduleVersionPtr)
        {
            var tokenSource = cancellationTokenSource as CancellationTokenSource;
            var cancellationToken = tokenSource?.Token ?? CancellationToken.None;

            return MethodExecutor.ExecuteMethod(command, new object[] { cancellationToken }, mdToken, moduleVersionPtr);
        }
    }
}
