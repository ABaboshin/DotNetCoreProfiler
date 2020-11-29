using Interception.Attributes;
using Interception.Attributes.Cache;
using Interception.Core;
using System;
using System.Linq;

namespace Interceptors
{
    /// <summary>
    /// cache interceptor
    /// </summary>
    [MethodInterceptorImplementation(typeof(SampleAttributes.MySampleAttribute))]
    public class MySampleInterceptor : BaseAttributedInterceptor
    {
        public override int Priority => 100;

        public override void ExecuteBefore()
        {
          var attribute = GetCustomAttribute<SampleAttributes.MySampleAttribute>();

          Console.WriteLine($"p1: {attribute.P1}");
          Console.WriteLine($"p2: {string.Join(", ", attribute.P2)}");
        }
    }
}
