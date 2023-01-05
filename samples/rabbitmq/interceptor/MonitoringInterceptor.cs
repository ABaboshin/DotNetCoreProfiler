// using Interception.Attributes;
// using Interception.Tracing;
// using OpenTracing.Util;
// using System.Linq;

// namespace Interception
// {
//     /// <summary>
//     /// monitor interceptor
//     /// </summary>
//     [MethodInterceptorImplementation(typeof(MonitorAttribute))]
//     public class MonitoringInterceptor : BaseMetricsInterceptor
//     {
//         public MonitoringInterceptor() : base(true)
//         {
//         }

//         public override int Priority => 0;

//         protected override void CreateScope()
//         {
//             var attribute = GetCustomAttribute<MonitorAttribute>();

//             var spanBuilder = GlobalTracer.Instance.BuildSpan(attribute.Name).AsChildOf(GlobalTracer.Instance.ActiveSpan);

//             if (attribute.Parameters != null && attribute.Parameters.Any())
//             {
//                 var methodParameters = Method.GetParameters().ToList();

//                 foreach (var p in attribute.Parameters)
//                 {
//                     var index = methodParameters.FindIndex(mp => mp.Name == p);
//                     if (index != -1)
//                     {
//                         spanBuilder = spanBuilder
//                             .WithTag($"parameter.{p}", GetParameter(index)?.ToString());
//                     }
//                 }
//             }

//             _scope = spanBuilder.StartActive();
//         }
//     }
// }
