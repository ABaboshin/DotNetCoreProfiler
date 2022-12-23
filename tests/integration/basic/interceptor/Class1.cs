using Interception.Attributes;
using Interception.Core;
using System;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M1", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
    public class C1
    {
      public static void Before(){
      Console.WriteLine($"Execute C1.Before");
      throw new Exception("break");
      }
      public static void After(){
      Console.WriteLine($"Execute C1.After");
      throw new Exception("break");
      }
    }

    // [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M11", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
    public class C11
    {
      public static void Before(){
      Console.WriteLine($"Execute C11.Before");
      throw new Exception("break");
      }
      public static void After(){
      Console.WriteLine($"Execute C11.After");
      throw new Exception("break");
      }
    }

  // [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M2", TargetTypeName = "app.C1", TargetMethodParametersCount = 1)]
  public class C2
  {
    public static void Before<T1>(T1 a1) {
      Console.WriteLine($"Execute C2.Before {a1} {typeof(T1)}");
    }
  }

  // [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M3", TargetTypeName = "app.C1", TargetMethodParametersCount = 0)]
  public class C3
  {
    public static void Before<T1>(T1 a1)
    {
      Console.WriteLine($"Execute C3.Before {a1} {typeof(T1)}");
    }
  }

  // [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "M4", TargetTypeName = "app.C1", TargetMethodParametersCount = 2)]
  public class C4
  {
    public static void Before<T1, T2, T3>(T1 a1, T2 a2, T3 a3)
    {
      Console.WriteLine($"Execute C4.Before {a1} {typeof(T1)}  {a2} {typeof(T2)}  {a3} {typeof(T3)}");
    }
  }
}
