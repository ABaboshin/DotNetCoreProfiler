﻿using System;
using Interception.Attributes;

namespace Interception.AspNetCore
{
    /// <summary>
    /// intercept WebHost.Run
    /// and get the DI
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "Microsoft.AspNetCore.Hosting", TargetMethodName = "Run", TargetTypeName = "Microsoft.AspNetCore.Hosting.WebHostExtensions", TargetMethodParametersCount = 1)]
    public class RunWebHostInterceptor
    {
        public static void Before<TType, T1>(TType instance, ref T1 webhost)
        {
            DependencyInjection.Instance.ServiceProvider = (IServiceProvider)webhost.GetType().GetProperty("Services").GetValue(webhost);
        }

        public static void After<TResult>(TResult result, Exception ex)
        {
        }
    }
}
