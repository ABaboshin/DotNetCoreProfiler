using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SampleApp
{
    public class TestC : ITestC
    {
        public string Data { get; set; } = "XXX";

        public override string ToString()
        {
            return $"TestC:{Data}";
        }

        public void TestVoid()
        {
            System.Console.WriteLine($"TestC.Test empty void");
        }

        public string Test()
        {
            System.Console.WriteLine($"TestC.Test empty string");
            return Guid.NewGuid().ToString();
        }

        public Task Test1Async()
        {
            System.Console.WriteLine($"TestC.Test1Async empty");
            return Task.CompletedTask;
        }

        public async Task<string> Test2Async()
        {
            System.Console.WriteLine($"TestC.Test2Async empty");
            await Task.Delay(100);
            return Guid.NewGuid().ToString();
        }
    }
}
