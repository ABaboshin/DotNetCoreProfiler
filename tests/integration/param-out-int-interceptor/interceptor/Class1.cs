using Interception.Attributes;
using Interception.Core;
using System;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "TestM", TargetTypeName = "app.Program", TargetMethodParametersCount = 1)]
    public class Class1 : BaseInterceptor
    {
        public override int Priority => 1;

        public override void ExecuteBefore()
        {
            Console.WriteLine(GetParameter(0));
        }

        public override void ExecuteAfter()
        {
          Console.WriteLine(GetParameter(0));
        }
    }
}
