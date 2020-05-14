using System;
using System.Threading.Tasks;

namespace Interception.Core.Extensions
{
    public static class TypeExtensions
    {
        public static bool IsReturnTypeTask(this Type returnType)
        {
            if (returnType == typeof(Task))
            {
                return true;
            }

            if (returnType.BaseType != null)
            {
                return IsReturnTypeTask(returnType.BaseType);
            }

            return false;
        }
    }
}
