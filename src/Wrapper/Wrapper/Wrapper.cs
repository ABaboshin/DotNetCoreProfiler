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
        private readonly Stopwatch _stopwatch = new Stopwatch();

        public void Finish(object returnValue, object exception)
        {
            _stopwatch.Stop();
            Console.WriteLine($"Wrapper.Finish {_stopwatch.ElapsedMilliseconds} {returnValue} {exception}");
        }

        public void Start()
        {
            _stopwatch.Start();
            Console.WriteLine("Wrapper.Start");
        }
    }
}
