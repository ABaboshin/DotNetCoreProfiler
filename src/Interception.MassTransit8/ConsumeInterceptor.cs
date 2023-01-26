using Interception.Attributes;
using MassTransit;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Linq;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System.Threading;
using Interception.Tracing.Extensions;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

namespace Interception.MassTransit
{
    /// <summary>
    /// intercept masstransit consumer
    /// and inject tracing
    /// </summary>
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1", TargetMethodParametersCount = 1)]
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.InitiatedBy`1", TargetMethodParametersCount = 1)]
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.Observes`2", TargetMethodParametersCount = 1)]
    [StrictIntercept(TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.Saga.Orchestrates`2", TargetMethodParametersCount = 1)]
    
    public class ConsumeInterceptor
    {
        protected static AsyncLocal<IScope> _scope = new AsyncLocal<IScope>();

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void Before<TType, TContext>(TType instance, ref TContext context) where TContext : ConsumeContext
        {
            if (!DependencyInjection.Instance.ServiceProvider.GetService<IConfiguration>().GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>().ConsumerEnabled)
            {
                return;
            }

            var consumerName = instance.GetType().FullName;

            ISpanBuilder spanBuilder;

            try
            {
                var headers = context.Headers.GetAll().ToDictionary(pair => pair.Key, pair => pair.Value.ToString());
                var parentSpanContext = GlobalTracer.Instance.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(headers));

                spanBuilder = GlobalTracer.Instance
                    .BuildSpan(DependencyInjection.Instance.ServiceProvider.GetService<IConfiguration>().GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>().ConsumerName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                    .AsChildOf(parentSpanContext);
            }
            catch (Exception)
            {
                spanBuilder = GlobalTracer.Instance.BuildSpan(DependencyInjection.Instance.ServiceProvider.GetService<IConfiguration>().GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>().ConsumerName);
            }

            spanBuilder
                .WithTag("Consumer", consumerName)
                .WithTag("MessageId", context.MessageId?.ToString());

            _scope.Value = spanBuilder.StartActive(true);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static void After<TResult>(ref TResult result, Exception ex) where TResult : Task
        {
            if (_scope.Value != null)
            {
                if (!result.IsCompleted && !result.IsCanceled)
                {
                    result.GetAwaiter().GetResult();
                }

                if (ex != null)
                {
                    _scope.Value.Span.SetException(ex);
                }

                _scope.Value.Span.SetTag("result", result?.ToString());

                _scope.Value.Dispose();
                _scope.Value = null;
            }
        }
    }
}
