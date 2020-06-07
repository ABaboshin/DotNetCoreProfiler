using System;
using System.Reflection;

namespace Interception.Core
{
    public interface IMethodFinder
    {
        MethodInfo FindMethod(int mdToken, long moduleVersionPtr, object obj, object[] parameters);
    }
}