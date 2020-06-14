using Interception.Attributes;
using Interception.Core;
using System;
using System.Linq;
using System.Reflection;

namespace Interception.MassTransit
{
    [MethodFinder(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1", TargetMethodParametersCount = 1)]
    [MethodFinder(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.InitiatedBy`1", TargetMethodParametersCount = 1)]
    [MethodFinder(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.Observes`2", TargetMethodParametersCount = 1)]
    [MethodFinder(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.Orchestrates`2", TargetMethodParametersCount = 1)]
    public class ConsumeMethodFinder : IMethodFinder
    {
        public MethodInfo FindMethod(int mdToken, long moduleVersionPtr, object obj, object[] parameters)
        {
            if (obj is null)
            {
                throw new ArgumentNullException("obj");
            }

            if (parameters is null)
            {
                throw new ArgumentNullException("parameters");
            }

            if (parameters.Count() != 1)
            {
                throw new ArgumentOutOfRangeException("parameters");
            }

            var interfaceType = parameters[0].GetType().GetInterfaces().Where(i => i.Name == "ConsumeContext`1").FirstOrDefault();
            if (interfaceType is null)
            {
                throw new ArgumentNullException("interfaceType");
            }

            var method = obj.GetType().GetMethods()
                .Where(m => m.Name == "Consume" && m.GetParameters().Count() == 1 && m.GetParameters()[0].ParameterType == interfaceType)
                .FirstOrDefault();

            if (method is null)
            {
                throw new ArgumentNullException("method");
            }

            return method;
        }
    }
}
