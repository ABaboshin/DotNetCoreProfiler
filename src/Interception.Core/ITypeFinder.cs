using System;

namespace Interception.Core
{
    public interface ITypeFinder
    {
        Type FindType(int mdToken, long moduleVersionPtr);
    }
}