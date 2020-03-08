using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SampleApp
{
    public class TestC : ITestC
    {
        public void Test()
        {
            System.Console.WriteLine($"TestC.Test empty");
        }
    }
}
