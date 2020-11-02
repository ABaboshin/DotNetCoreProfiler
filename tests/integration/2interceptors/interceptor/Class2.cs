using Interception.Attributes;
using Interception.Core;
using System;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "TestM", TargetTypeName = "app.Program", TargetMethodParametersCount = 0)]
    public class Class2 : BaseInterceptor
    {
        public override int Priority => 2;

        public override void ExecuteBefore()
        {
            Console.WriteLine("ExecuteBefore2");
        }

        public override void ExecuteAfter()
        {
            Console.WriteLine("ExecuteAfter2");
        }
    }
}
