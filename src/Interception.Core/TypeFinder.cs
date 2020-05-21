using System;
using System.Runtime.InteropServices;

namespace Interception.Core
{
    public class TypeFinder : ITypeFinder
    {
        public Type FindType(int mdToken, long moduleVersionPtr)
        {
            var ptr = new IntPtr(moduleVersionPtr);
            var moduleVersionId = Marshal.PtrToStructure<Guid>(ptr);

            //var type = System.Type.GetTypeFromHandle(mdToken);

            var assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (var assembly in assemblies)
            {
                foreach (var module in assembly.Modules)
                {
                    if (module.ModuleVersionId == moduleVersionId)
                    {
                        try
                        {
                            return module.ResolveType(mdToken);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"Exception {ex}");
                        }
                    }
                }
            }

            Console.WriteLine($"Type not found");

            return null;
        }
    }
}