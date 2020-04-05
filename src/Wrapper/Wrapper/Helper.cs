using System;
using System.Collections.Generic;
using System.Text;

namespace Wrapper
{
    class Helper
    {
        void Test1(string s1, int i1, object o1 = null)
        {
            var data = new[] { s1, i1, o1 };
        }

        void Test2()
        {
            Test1("x", 1);
        }
    }
}
