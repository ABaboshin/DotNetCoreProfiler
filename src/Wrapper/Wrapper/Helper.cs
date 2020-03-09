using System;
using System.Collections.Generic;
using System.Text;

namespace Wrapper
{
    public class Helper
    {
        public Helper()
        {
            //var ar = new object[3];
            //ar[0] = 1;
            //ar[1] = "test";
            //ar[2] = new { x = 3 };
            //Wrapper.Test(ar);
            var test = new Wrapper();
            test.Start();
            test.Finish(null, null);
            //GetInstance().BeforeMethod(new object(), new object(), new[] { new object(), 1 }, 3);
        }

        private static readonly Helper Instance = new Helper();

        public static Helper GetInstance()
        {
            return Instance;
        }

        public object BeforeMethod(object type, object invocationTarget, object[] methodArguments, uint functionToken)
        {
            return new Wrapper();
        }
    }
}
