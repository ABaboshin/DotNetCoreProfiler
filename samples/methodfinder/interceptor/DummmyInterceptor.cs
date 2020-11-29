using Interception.Attributes;
using Interception.Core;
using System;
using System.Linq;
using System.Reflection;

namespace interceptor
{
    [StrictIntercept(TargetAssemblyName = "app", TargetMethodName = "TestM", TargetTypeName = "app.TC`1", TargetMethodParametersCount = 1)]
    public class DummmyInterceptor : BaseInterceptor
    {
        public override int Priority => 1;

        public override void ExecuteBefore()
        {
        }

        public override void ExecuteAfter()
        {
        }
    }
}
