using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using System;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {
        public string Test { get; set; }

        public static void Main(string[] args)
        {
            Action act = () => { Console.WriteLine(args); };

            TestX();
            //int i = 3;
            //int k = 5;
            //var p1 = new Program();
            //var p2 = new Program();
            //TestM(i, ref k, p1, ref p2);
            CreateWebHostBuilder(args).Build().Run();
        }

        static async Task TestA()
        {
            await Task.Delay(10000);
        }

        static async Task<T> TestA2<T>(T t)
        {
            await Task.Delay(100);
            return t;
        }

        static async Task TestX()
        {
            //var test = TestA();
            //var awaiter = test.GetAwaiter();
            //var builder = AsyncTaskMethodBuilder.Create();
            //if (!awaiter.IsCompleted)
            //{
            //    var sm = new StateMachine();
            //    builder.AwaitUnsafeOnCompleted(ref awaiter, ref sm);
            //}

            //return test;



            ////await TestA();
            var t1 = await TestA2(3);

            Console.WriteLine("test");
            //var t2 = await TestA2(new Program());
        }

        public static void TestM(int i, ref int k, Program p1, ref Program p2)
        {
            i = 1;
            k = 2;
            p1 = new Program { Test = "123" };
            p2 = new Program { Test = "abcd" };
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
