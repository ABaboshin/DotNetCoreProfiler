using Interception.Common;
using Interception.Common.Extensions;
using Interception.Tracing.Extensions;
using Microsoft.Extensions.Configuration;
using OpenTracing.Util;
using Newtonsoft.Json;
using OpenTracing;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using StackExchange.Redis;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Interception.StackExchangeRedis
{
    public static class StackExchangeRedisInterception
    {
        public static RedisConfiguration RedisConfiguration;

        public static void Configure()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            RedisConfiguration = configuration.GetSection(RedisConfiguration.SectionKey).Get<RedisConfiguration>();
        }

        [Intercept(CallerAssembly = "SampleApp", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "GetSubscriber", TargetTypeName = "StackExchange.Redis.IConnectionMultiplexer", TargetMethodParametersCount = 1)]
        public static object GetSubscriber(object multiplexer, object asyncState, int mdToken, long moduleVersionPtr)
        {
            if (RedisConfiguration.Enabled)
            {
                var result = MethodExecutor.ExecuteMethod(multiplexer, new[] { asyncState }, mdToken, moduleVersionPtr, noMetrics: true);
                return new InterceptionSubscriber(result as ISubscriber);
            }
            else
            {
                return MethodExecutor.ExecuteMethod(multiplexer, new[] { asyncState }, mdToken, moduleVersionPtr, noMetrics: true);
            }
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "ForInvoke", TargetTypeName = "StackExchange.Redis.ConnectionMultiplexer.Subscription", TargetMethodParametersCount = 2)]
        public static object ForInvoke(object subscription, object channel, object message, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"ForInvoke {subscription} {channel} {message}");
            return null;
            //if (RedisConfiguration.Enabled)
            //{
            //    var result = MethodExecutor.ExecuteMethod(multiplexer, new[] { asyncState }, mdToken, moduleVersionPtr, noMetrics: true);
            //    return new InterceptionSubscriber(result as ISubscriber);
            //}
            //else
            //{
            //    return MethodExecutor.ExecuteMethod(multiplexer, new[] { asyncState }, mdToken, moduleVersionPtr, noMetrics: true);
            //}
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "TryComplete", TargetTypeName = "StackExchange.Redis.ICompletable", TargetMethodParametersCount = 1)]
        public static bool TryComplete(object messageCompletable, bool isAsync, int mdToken, long moduleVersionPtr)
        {
            if (messageCompletable.GetType().Name == "MessageCompletable")
            {
                messageCompletable.TryGetFieldValue("channel", out object channel);
                messageCompletable.TryGetFieldValue("syncHandler", out object syncHandler);
                messageCompletable.TryGetFieldValue("asyncHandler", out object asyncHandler);

                Console.WriteLine($"MessageCompletable syncHandler {syncHandler} asyncHandler {asyncHandler} isAsync {isAsync}");

                var result = MethodExecutor.ExecuteMethod(
                    messageCompletable,
                    new object[] { isAsync },
                    mdToken,
                    moduleVersionPtr,
                    !RedisConfiguration.Enabled,
                    RedisConfiguration.ConsumerName,
                    new Dictionary<string, string> { { "channel", channel.ToString() } });

                return (bool)result;
            }
            else
            {
                return (bool)MethodExecutor.ExecuteMethod(messageCompletable, new object[] { isAsync }, mdToken, moduleVersionPtr, true);
            }
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "OnMessageSyncImpl", TargetTypeName = "StackExchange.Redis.ChannelMessageQueue", TargetMethodParametersCount = 0)]
        public static object OnMessageSyncImpl(object channelMessageQueue, int mdToken, long moduleVersionPtr)
        {
            Console.WriteLine($"OnMessageSyncImpl {channelMessageQueue}");
            return OnMessageImpl(channelMessageQueue, mdToken, moduleVersionPtr, (handler, next) => {
                handler.DynamicInvoke(next);
                return Task.CompletedTask;
            });
        }

        //[Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "OnMessageAsyncImpl", TargetTypeName = "StackExchange.Redis.ChannelMessageQueue", TargetMethodParametersCount = 0)]
        public static async Task OnMessageAsyncImpl(object channelMessageQueue, int mdToken, long moduleVersionPtr)
        {
            await OnMessageImpl(channelMessageQueue, mdToken, moduleVersionPtr, async (handler, next) => {
                await (Task)handler.DynamicInvoke(next);
            });
        }

        private static async Task OnMessageImpl(object cmq, int mdToken, long moduleVersionPtr, Func<Delegate, object, Task> executor)
        {
            var channelMessageQueue = (ChannelMessageQueue)cmq;

            channelMessageQueue.TryGetFieldValue("_onMessageHandler", out Delegate handler);

            while (!channelMessageQueue.Completion.IsCompleted)
            {
                Console.WriteLine("while");
                try
                {
                    if (!channelMessageQueue.TryRead(out ChannelMessage channelMessage))
                    {
                        Console.WriteLine("!TryRead");
                        channelMessage = await channelMessageQueue.ReadAsync().ForAwait();
                        Console.WriteLine("!TryRead done");
                    }
                    else
                    {
                        Console.WriteLine("TryRead");
                    }

                    if (!RedisConfiguration.Enabled)
                    {
                        await executor(handler, channelMessage);
                    }
                    else
                    {
                        Console.WriteLine($"Message {channelMessage.Message}");
                        var interceptionMessage = JsonConvert.DeserializeObject<InterceptionMessage>(channelMessage.Message);

                        var type = typeof(ChannelMessage);
                        var ctors = type.GetConstructors(System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
                        var unwrappedMessage = ctors[0].Invoke(new object[] { channelMessageQueue, channelMessage.Channel, (RedisValue)interceptionMessage.Message });

                        ISpanBuilder spanBuilder;

                        try
                        {
                            var parentSpanContext = GlobalTracer.Instance.Extract(BuiltinFormats.TextMap, new TextMapExtractAdapter(interceptionMessage.Tracing));

                            spanBuilder = GlobalTracer.Instance
                                .BuildSpan(RedisConfiguration.ConsumerName)
                                .WithTag(Tags.SpanKind, Tags.SpanKindConsumer)
                                .AsChildOf(parentSpanContext);
                            Console.WriteLine("aschildof");
                        }
                        catch (Exception)
                        {
                            Console.WriteLine("new trace");
                            spanBuilder = GlobalTracer.Instance.BuildSpan(RedisConfiguration.ConsumerName);
                        }

                        spanBuilder
                            .WithTag("channel", channelMessage.Channel);

                        using (var scope = GlobalTracer.Instance.BuildSpan(RedisConfiguration.ConsumerName).AsChildOf(GlobalTracer.Instance.ActiveSpan).StartActive())
                        {
                            try
                            {
                                await executor(handler, unwrappedMessage);
                            }
                            catch (Exception ex)
                            {
                                GlobalTracer.Instance.ActiveSpan.SetException(ex);
                                throw;
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Ooops {ex.Message}");
                    if (ex.GetType().Name == "ChannelClosedException")
                    {
                        Console.WriteLine("ChannelClosedException");
                        break;
                    }

                    channelMessageQueue.TryGetPropertyValue("_parent", out object _parent);
                    _parent.TryGetPropertyValue("multiplexer", out object multiplexer);

                    if (multiplexer != null)
                    {
                        var OnInternalError = multiplexer.GetType().GetMethod("OnInternalError");
                        if (OnInternalError != null)
                        {
                            OnInternalError.Invoke(multiplexer, new[] { ex });
                        }
                    }
                }
            }

            /*var handler = (Action<ChannelMessage>)_onMessageHandler;
            while (!Completion.IsCompleted)
            {
                ChannelMessage next;
                try { if (!TryRead(out next)) next = await ReadAsync().ForAwait(); }
                catch (ChannelClosedException) { break; } // expected
                catch (Exception ex)
                {
                    _parent.multiplexer?.OnInternalError(ex);
                    break;
                }

                try { handler(next); }
                catch { } // matches MessageCompletable
            }*/
        }
    }
}
