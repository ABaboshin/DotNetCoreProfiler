﻿using Interception.Attributes;
using MassTransit;
using Microsoft.Extensions.Options;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Linq;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Interception.Tracing;

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
    public class ConsumeInterceptor : BaseMetricsInterceptor
    {
        public ConsumeInterceptor() : base(DependencyInjection.Instance.ServiceProvider.GetService<IConfiguration>().GetSection(MassTransitConfiguration.SectionKey).Get<MassTransitConfiguration>().ConsumerEnabled)
        {
        }

        public override int Priority => 0;

        protected override void CreateScope()
        {
            var context = (ConsumeContext)GetParameter(0);
            var consumerName = This.GetType().FullName;

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

            _scope = spanBuilder.StartActive(true);
        }
    }
}
