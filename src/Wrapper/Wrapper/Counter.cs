using StatsdClient;
using System;
using System.Runtime.InteropServices;

namespace Wrapper
{
    public class Counter
    {
        static object lck = new object();
        static bool configured = false;

        public static void Count(string functionName, int mdToken, long moduleVersionPtr)
        {
            lock (lck)
            {
                if (!configured)
                {
                    DogStatsd.Configure(new StatsdConfig
                    {
                        StatsdServerName = "statsd",
                        StatsdPort = 9125
                    });

                    configured = true;
                }
            }

            var ptr = new IntPtr(moduleVersionPtr);
            var moduleVersionId = Marshal.PtrToStructure<Guid>(ptr);

            //Console.WriteLine($"Call to {functionName}, {mdToken}, {moduleVersionPtr}, {moduleVersionId}");

            var found = false;
            var assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (var assembly in assemblies)
            {
                foreach (var module in assembly.Modules)
                {
                    if (module.ModuleVersionId == moduleVersionId)
                    {
                        try
                        {
                            var type = module.ResolveMethod(mdToken);
                            Console.WriteLine($"Found {type.DeclaringType.Name}.{type.Name} Call to {functionName}, {mdToken}, {moduleVersionPtr}, {moduleVersionId}");
                            found = true;
                        }
                        catch (Exception)
                        {
                        }
                    }
                    //Console.WriteLine($"{assembly.FullName} {module.ModuleVersionId}");
                }
            }

            if (!found)
            {
                Console.WriteLine($"Not found call to {functionName}");
            }

            //DogStatsd.Counter("function_call", 1, tags: new[] { $"name:{functionName}" });

            //Console.WriteLine($"Call to {functionName}");
        }
    }
}
