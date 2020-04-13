using Interception.Common;
using Interception.Common.Extensions;
using System;
using System.Collections.Generic;

namespace Interception
{
    public static class StackExchangeRedisInterception
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "TryComplete", TargetTypeName = "StackExchange.Redis.ICompletable", TargetMethodParametersCount = 1)]
        public static bool TryComplete(object messageCompletable, bool isAsync, int mdToken, long moduleVersionPtr)
        {
            if (messageCompletable.GetType().Name == "MessageCompletable")
            {
                messageCompletable.TryGetFieldValue("channel", out object channel);

                return (bool)MethodExecutor.ExecuteMethod(messageCompletable, new object[] { isAsync }, mdToken, moduleVersionPtr, false, "redis_call", new List<string> { $"channel:{channel}" });
            }
            else
            {
                return (bool)MethodExecutor.ExecuteMethod(messageCompletable, new object[] { isAsync }, mdToken, moduleVersionPtr, true);
            }
        }
    }
}
