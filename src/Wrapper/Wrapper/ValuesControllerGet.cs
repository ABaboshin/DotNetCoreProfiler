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

                            var result = method.Invoke(controller, null);
                            Console.WriteLine($"result {result}");
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
                Console.WriteLine($"Not found call");
            }

            //return null;
        }
    }
}
