using Interception.Attributes;
using Interception.Core.Interop;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Interception.Core
{
    [ComposeInterceptor]
    public class ComposeInterceptor : BaseInterceptor
    {
        private List<IInterceptor> _interceptors = new List<IInterceptor>();

        public override int Priority => 0;

        public override void ExecuteAfter(object result, Exception exception)
        {
            foreach (var item in _interceptors.OrderByDescending(x => x.Priority))
            {
                Console.WriteLine($"ExecuteAfter {item.GetType().Name}");
                item.ExecuteAfter(result, exception);
            }
        }

        public override void ExecuteBefore()
        {
            foreach (var item in _interceptors.OrderBy(x => x.Priority))
            {
                Console.WriteLine($"ExecuteBefore {item.GetType().Name}");
                item.ExecuteBefore();
            }
        }

        public override object Execute()
        {
            var interceptors = NativeMethods.GetInterceptions(GetKey());
            foreach (var item in interceptors)
            {
                var assembly = AppDomain.CurrentDomain.GetAssemblies().Where(a => a.GetName().Name == item.AssemblyName).FirstOrDefault();
                if (assembly is null)
                {
                    continue;
                }

                var type = assembly.GetType(item.TypeName);
                if (type is null)
                {
                    continue;
                }

                var interceptor = (IInterceptor)Activator.CreateInstance(type);
                interceptor.SetThis(GetThis());
                interceptor.SetModuleVersionPtr(GetModuleVersionPtr());
                interceptor.SetParameters(GetParameters());
                interceptor.SetMdToken(GetMdToken());

                _interceptors.Add(interceptor);
            }

            return base.Execute();
        }
    }
}
