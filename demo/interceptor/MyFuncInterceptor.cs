using System;
using Interception.Attributes;
using Interception.Core;

namespace interceptor
{
    [StrictIntercept(
        TargetAssemblyName = "app",
        TargetMethodName = "MyFunc",
        TargetTypeName = "app.Program",
        TargetMethodParametersCount = 1)]
    public class MyFuncInterceptor : BaseInterceptor
    {
        public override int Priority => 0;

        public override void ExecuteBefore()
        {
            Console.WriteLine("ExecuteBefore");
            Console.WriteLine("This is null ", This is null);
            Console.WriteLine("First parameter ", GetParameter(0));
        }

        public override void ExecuteAfter()
        {
            Console.WriteLine("ExecuteBefore");
        }
    }
}
