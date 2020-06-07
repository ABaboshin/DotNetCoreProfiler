using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Interception.Core
{
    public class DefaultMethodFinder : IMethodFinder
    {
        public MethodInfo FindMethod(int mdToken, long moduleVersionPtr, object obj, object[] parameters)
        {
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
                            return (MethodInfo)module.ResolveMethod(mdToken);
                        }
                        catch (Exception)
                        {
                        }
                    }
                }
            }

            return null;
        }
    }
}
