using System;
using System.Collections.Generic;
using System.Text;

namespace Interception.Cache
{
    public interface IDistributedCacheInvalidator
    {
        void Invalidate(string prefix);
    }
}
