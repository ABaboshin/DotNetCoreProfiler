using System;
using System.Collections.Generic;
using System.Text;

namespace Wrapper
{
    public class Helper
    {
        public Helper()
        {
            Wrapper.Test(null);
            var test = new Wrapper();
            test.Start();
            test.Finish(null, null);
        }

        private static readonly Helper Instance = new Helper();

        public static object GetInstance()
        {
            return Instance;
        }

        public object BeforeMethod(object type, object invocationTarget, object[] methodArguments, uint functionToken)
        {
            return null;
        }
    }
}
