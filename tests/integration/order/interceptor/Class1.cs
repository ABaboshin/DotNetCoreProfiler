using Interception.Attributes;
using Interception.Core;
using System;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "TestM", TargetTypeName = "app.Program", TargetMethodParametersCount = 0)]
    public class Class1 : BaseInterceptor
    {
        public override int Priority => 1;

        public override void ExecuteBefore()
        {
            Console.WriteLine("ExecuteBefore1");
        }

        public override void ExecuteAfter()
        {
            Console.WriteLine("ExecuteAfter1");
        }
    }
}
