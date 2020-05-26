using Interception.Attributes;
using Interception.Core;

namespace Interception.Serilog
{
    [StrictIntercept(
        TargetAssemblyName = "Serilog.AspNetCore",
        TargetMethodName = "UseSerilog",
        TargetTypeName = "Serilog.SerilogWebHostBuilderExtensions",
        TargetMethodParametersCount = 3)]
    public class DisableUseSerilog : BaseInterceptor
    {
        public override int Priority => 0;

        public override bool SkipExecution()
        {
            Result = GetParameter(0);
            return true;
        }
    }
}
