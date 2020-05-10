using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Interception.Base
{
    public class MethodFinder : IMethodFinder
    {
        public MethodBase FindMethod(int mdToken, long moduleVersionPtr, Type[] genericTypeArguments = null)
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
                            MethodBase method = null;

                            if (genericTypeArguments != null)
                            {
                                method = module.ResolveMethod(mdToken, genericTypeArguments, new Type[] { });
                            }
                            else
                            {
                                method = module.ResolveMethod(mdToken);
                            }

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
