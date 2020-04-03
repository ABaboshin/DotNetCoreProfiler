using System;
using System.Collections.Generic;
using System.Text;

namespace Wrapper
{
    public class Helper
    {
        public Helper()
        {
            var test = new Wrapper();
            test.Test(1, 2, 3, 4, 5, 6, 7, 8);
            test.Start();
            try
            {
                throw new NotImplementedException();
            }
            catch (Exception ex)
            {
                test.Finish(null, ex);
                throw;
            }
        }

        public string Sample(int p1, string s1)
        {
            var test = new Wrapper();
            
            try
            {
                test.Start();
                //throw new NotImplementedException();
                var res = SampleInternal(p1, s1);
                test.Finish(res, null);
                return res;
            }
            catch (Exception ex)
            {
                test.Finish(null, ex);
                throw;
            }
        }

        public string SampleInternal(int p1, string s1)
        {
            return "test";
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
