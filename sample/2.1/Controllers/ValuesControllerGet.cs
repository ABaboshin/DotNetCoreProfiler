using System;

namespace SampleApp.Controllers
{
    internal class ValuesControllerGet
    {
        internal static Guid Replace(object p, string v1, int v2, long v3)
        {
            Console.WriteLine($"Repalce {p} {v1} {v2} {v3}");
            return Guid.NewGuid();
        }
    }
}