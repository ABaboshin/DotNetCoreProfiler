using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Wrapper.Common
{
    public static class MethodFinder
    {
        public static MethodBase FindMethod(int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"Call FindMethod {mdToken} {moduleVersionPtr}");

            var ptr = new IntPtr(moduleVersionPtr);
            var moduleVersionId = Marshal.PtrToStructure<Guid>(ptr);

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
                            Console.WriteLine($"Found {method.DeclaringType.Name}.{method.Name} Call, {mdToken}, {moduleVersionPtr}, {moduleVersionId}");

                            return method;
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"Exception {ex}");
                        }
                    }
                }
            }

            return null;
        }
    }
}
