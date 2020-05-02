using System;
using System.Threading.Tasks;

namespace Interception.Base.Extensions
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
