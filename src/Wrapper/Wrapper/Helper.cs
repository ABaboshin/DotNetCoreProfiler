using System;
using System.Collections.Generic;
using System.Text;

namespace Wrapper
{
    public class Helper
    {
        public Helper()
        {

        }

        private static readonly Helper Instance = new Helper();

        public static object GetInstance()
        {
            return Instance;
        }
    }
}
