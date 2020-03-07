using System;
using System.Diagnostics;

namespace Wrapper
{
    public interface IWrapper
    {
        void Start();
        void Finish(object returnValue, object exception);
    }

    public class Wrapper : IWrapper
    {
        public static void Test()
        {
            Console.WriteLine("Wrapper.Test 42");
        }

        //private readonly Stopwatch _stopwatch = new Stopwatch();

        //public Wrapper()
        //{
        //    Console.WriteLine("Wrapper.ctor");
        //}

        public void Finish(object returnValue, object exception)
        {
            //_stopwatch.Stop();
            //Console.WriteLine($"Wrapper.Finish {_stopwatch.ElapsedMilliseconds} {returnValue} {exception}");
        }

        public void Start()
        {
            //_stopwatch.Start();
            //Console.WriteLine("Wrapper.Start");
        }
    }
}
