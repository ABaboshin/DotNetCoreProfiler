using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Wrapper
{
    public class ValuesControllerGet
    {
        public static void Replace(object controller, string functionName, int mdToken, long moduleVersionPtr)
        {
            var ptr = new IntPtr(moduleVersionPtr);
            var moduleVersionId = Marshal.PtrToStructure<Guid>(ptr);

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
                            var method = module.ResolveMethod(mdToken);
                            Console.WriteLine($"Found {method.DeclaringType.Name}.{method.Name} Call, {mdToken}, {moduleVersionPtr}, {moduleVersionId}, {functionName}");
                            found = true;

                            Console.WriteLine("Start calling");
                            var result = method.Invoke(controller, new object[] { });

                            //var controllerType = Type.GetType("SampleApp.Controllers.ValuesController, SampleApp");



                            //Console.WriteLine(controllerType);

                            Console.WriteLine("Finish calling");
                            //Console.WriteLine($"result {result}");
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"Exception {ex}");
                        }
                    }
                    //Console.WriteLine($"{assembly.FullName} {module.ModuleVersionId}");
                }
            }

            if (!found)
            {
                Console.WriteLine($"Not found call");
            }

            //return null;
        }

        public static Guid Replace2(object p, string v1, int v2, long v3, string functionName, int mdToken, long moduleVersionPtr)
        {
            var ptr = new IntPtr(moduleVersionPtr);
            var moduleVersionId = Marshal.PtrToStructure<Guid>(ptr);

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
                            var method = module.ResolveMethod(mdToken);
                            Console.WriteLine($"Found2 {method.DeclaringType.Name}.{method.Name} Call, {mdToken}, {moduleVersionPtr}, {moduleVersionId}, {functionName}");
                            found = true;

                            Console.WriteLine($"Start2 calling {p} {v1} {v2} {v3}");
                            var result = method.Invoke(null, new[] { p, v1, v2, v3 });
                            Console.WriteLine("Finish2 calling");
                            Console.WriteLine($"result {result}");

                            return (Guid)result;
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"Exception2 {ex}");
                        }
                    }
                    //Console.WriteLine($"{assembly.FullName} {module.ModuleVersionId}");
                }
            }

            if (!found)
            {
                Console.WriteLine($"Not found call");
            }

            return Guid.Empty;
        }
    }
}
