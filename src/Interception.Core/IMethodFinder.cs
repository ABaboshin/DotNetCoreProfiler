using System;
using System.Reflection;

namespace Interception.Core
{
    public interface IMethodFinder
    {
        MethodBase FindMethod(int mdToken, long moduleVersionPtr, Type[] genericTypeArguments = null);
    }
}