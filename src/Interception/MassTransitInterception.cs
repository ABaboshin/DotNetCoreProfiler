using Interception.Common;
using MassTransit;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace Interception
{
    public static class MassTransitInterception
    {
        //[Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1[!1]", TargetMethodParametersCount = 1)]
        public static Task Consume<T>(/*IConsumer<T>*/object consumer, /*ConsumeContext<T>*/object content, int mdToken, long moduleVersionPtr)
            //where T : class
        {
            Console.WriteLine(moduleVersionPtr);
            Console.WriteLine(mdToken);
            Console.WriteLine(content);
            Console.WriteLine(consumer);
            Console.WriteLine($"TConsumer {consumer.GetType().Name} T {typeof(T).Name}");

            return (Task)MethodExecutor.ExecuteMethod(consumer, new object[] { content }, mdToken, moduleVersionPtr);
        }

        //[Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Publish", TargetTypeName = "MassTransit.IPublishEndpoint", TargetMethodParametersCount = 1)]
        public static Task Publish<T>(object bus, T message, int mdToken, long moduleVersionPtr)
        {
            return (Task)MethodExecutor.ExecuteMethod(bus, new object[] { message, null }, mdToken, moduleVersionPtr);
        }
    }
}
