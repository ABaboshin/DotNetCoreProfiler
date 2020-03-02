using System;
using System.Reflection;

namespace Wrapper
{
    public class Wrapper
    {
        public Wrapper()
        {
            Console.WriteLine("Wrapper.ctor");
        }

        public static void Wrap()
        {
            Console.WriteLine("Start Wrap");
            try
            {
                Assembly.LoadFile("test");
                System.Console.WriteLine("test");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"catch {ex.Message}");
                throw;
            }
            Console.WriteLine("Finish Wrap");
        }
    }
}
