using Interception.Attributes;
using Interception.Core;
using System;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M1", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
    public class M1
    {
        public static void Before<T>(T obj)
        {
            Console.WriteLine($"Execute M1.Before {Environment.StackTrace} {obj != null} {typeof(T).FullName}");
            throw new Exception("break");
        }
        public static void After<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"Execute M1.After {Environment.StackTrace}");
            throw new Exception("break");
        }
    }

    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M11", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
    public class M11
    {
        public static void Before<T>(T obj)
        {
            Console.WriteLine($"Execute M11.Before {Environment.StackTrace}");
            throw new Exception("break");
        }
        public static void After<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"Execute M11.After");
            throw new Exception("break");
        }
    }

    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M2", TargetTypeName = "app.C1", TargetMethodParametersCount = 1)]
    public class M2
    {
        public static void Before<TType, T1>(TType instance, T1 a1)
        {
            Console.WriteLine($"Execute M2.Before {a1} {typeof(T1)} instance {instance != null} {typeof(TType).FullName}");
            throw new Exception("break");
        }
        public static void After<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"Execute M2.After result {result != null} {typeof(TResult).FullName} exception {ex != null}");
            throw new Exception("break");
        }
    }

    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M3", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
    public class M3
    {
        public static void Before<T1>(T1 a1)
        {
            Console.WriteLine($"Execute M3.Before {a1} {typeof(T1)}");
            throw new Exception("break");
        }

        public static void After<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"Execute M3.After result {result != null} {typeof(TResult).FullName} exception {ex != null}");
            throw new Exception("break");
        }
    }

    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M4", TargetTypeName = "app.C1", TargetMethodParametersCount = 2)]
    public class M4
    {
        public static void Before<TType, T1, T2>(TType instance, T1 a1, T2 a2)
        {
            Console.WriteLine($"Execute M4.Before instance != null {instance != null} of type {typeof(TType).FullName} params {a1} {typeof(T1)} {a2} {typeof(T2)}");
            throw new Exception("from before");
        }

        public static void After<TResult>(TResult result, Exception ex)
        {
            Console.WriteLine($"Execute M4.After result {result != null} {typeof(TResult).FullName} exception {ex != null}");
            throw new Exception("from after");
        }
    }
}
