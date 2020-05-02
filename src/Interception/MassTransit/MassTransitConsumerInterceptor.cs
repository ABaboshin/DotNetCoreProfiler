﻿using Interception.AspNetCore;
using Interception.Attributes;
using Interception.Base;
using MassTransit;
using Microsoft.Extensions.Options;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Linq;
using System.Reflection;
using Microsoft.Extensions.DependencyInjection;

namespace Interception.MassTransit
{
    [Intercept(CallerAssembly = "", TargetAssemblyName = "MassTransit", TargetMethodName = "Consume", TargetTypeName = "MassTransit.IConsumer`1", TargetMethodParametersCount = 1)]
    public class MassTransitConsumerInterceptor : BaseInterceptor
    {
        public override object Execute()
        {
            var options = DependencyInjection.ServiceProvider.GetService<IOptions<MassTransitConfiguration>>();

            Console.WriteLine($"options {options.Value}");

            return ExecuteInternal(options.Value.ConsumerEnabled);
        }

        protected override MethodInfo FindMethod()
        {
            return _this.GetType().GetMethod("Consume");
        }

        protected override IScope CreateScope()
        {
            var context = (ConsumeContext)_parameters[0];
            var consumerName = _this.GetType().FullName;

            ISpanBuilder spanBuilder;

            try
            {
                var headers = context.Headers.GetAll().ToDictionary(pair => pair.Key, pair => pair.Value.ToString());
                var parentSpanContext = GlobalTracer.Instance.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(headers));

                spanBuilder = GlobalTracer.Instance
                    .BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<MassTransitConfiguration>>().Value.ConsumerName)
                    .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                    .AsChildOf(parentSpanContext);
            }
            catch (Exception)
            {
                spanBuilder = GlobalTracer.Instance.BuildSpan(DependencyInjection.ServiceProvider.GetService<IOptions<MassTransitConfiguration>>().Value.ConsumerName);
            }

            spanBuilder
                .WithTag("consumer", consumerName)
                .WithTag("message-id", context.MessageId?.ToString());

            return spanBuilder.StartActive(true);
        }
    }
}
