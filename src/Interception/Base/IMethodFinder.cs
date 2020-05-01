using System;
using System.Reflection;

namespace Interception.Base
{
    public interface IMethodFinder
    {
        MethodBase FindMethod(int mdToken, long moduleVersionPtr, Type[] genericTypeArguments = null);
    }
}