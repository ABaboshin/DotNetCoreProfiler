using Interception.Attributes;
using Interception.Core;
using System;
using System.Linq;
using System.Reflection;

namespace interceptor
{
  [MethodFinder(TargetAssemblyName = "app", TargetMethodName = "TestM", TargetTypeName = "app.TC`1", TargetMethodParametersCount = 1)]
  public class TestMethodFinder : IMethodFinder
  {
    static bool firstCall = true;
    public MethodInfo FindMethod(int mdToken, long moduleVersionPtr, object obj, object[] parameters)
    {
      var method = obj.GetType().GetMethods()
          .Where(m => m.Name == "TestM" && m.GetParameters().Count() == 1)
          .FirstOrDefault();

      if (method is null)
      {
        if (firstCall)
        {
          firstCall = false;
          Console.WriteLine("not found");
        }

        throw new ArgumentNullException("method");
      }

      if (firstCall)
      {
        firstCall = false;
        Console.WriteLine("found");
      }

      return method;
    }
  }
}
