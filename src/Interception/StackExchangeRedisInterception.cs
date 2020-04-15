using Interception.Common;
using Interception.Common.Extensions;
using Interception.Metrics;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Interception
{
    public static class StackExchangeRedisInterception
    {
        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "TryComplete", TargetTypeName = "StackExchange.Redis.ICompletable", TargetMethodParametersCount = 1)]
        public static bool TryComplete(object messageCompletable, bool isAsync, int mdToken, long moduleVersionPtr)
        {
            if (messageCompletable.GetType().Name == "MessageCompletable")
            {
                messageCompletable.TryGetFieldValue("channel", out object channel);

                var result = MethodExecutor.ExecuteMethod(messageCompletable, new object[] { isAsync }, mdToken, moduleVersionPtr, false, "redis_call", new List<string> { $"channel:{channel}" });

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
            return OnMessageImpl(channelMessageQueue, mdToken, moduleVersionPtr, (handler, next) => {
                handler.DynamicInvoke(next);
            });
        }

        [Intercept(CallerAssembly = "", TargetAssemblyName = "StackExchange.Redis", TargetMethodName = "OnMessageAsyncImpl", TargetTypeName = "StackExchange.Redis.ChannelMessageQueue", TargetMethodParametersCount = 0)]
        public static object OnMessageAsyncImpl(object channelMessageQueue, int mdToken, long moduleVersionPtr)
        {
            return OnMessageImpl(channelMessageQueue, mdToken, moduleVersionPtr, (handler, next) => {
                var task = (Task)handler.DynamicInvoke(next);
                task.Wait();
            });
        }

        private static object OnMessageImpl(object channelMessageQueue, int mdToken, long moduleVersionPtr, Action<Delegate, object> executor)
        {
            channelMessageQueue.TryGetPropertyValue("Channel", out object channelProp);
            channelMessageQueue.TryGetFieldValue("_onMessageHandler", out object _onMessageHandler);

            var handler = (Delegate)_onMessageHandler;

            channelMessageQueue.TryGetPropertyValue("Completion", out Task Completion);

            while (!Completion.IsCompleted)
            {
                try
                {
                    var TryRead = channelMessageQueue.GetType().GetMethod("TryRead");
                    var channelMessageType = Type.GetType("StackExchange.Redis.ChannelMessage, StackExchange.Redis, Version=2.0.0.0, Culture=neutral, PublicKeyToken=c219ff1ca8c2ce46");
                    object next = null;
                    var parameters = new object[] { /*next*/null };
                    var tryReadResult = (bool)TryRead.Invoke(channelMessageQueue, parameters);
                    if (!tryReadResult)
                    {
                        var ReadAsync = channelMessageQueue.GetType().GetMethod("ReadAsync");
                        var readAsyncCall = (Task)ReadAsync.Invoke(channelMessageQueue, new object[] { });
                        readAsyncCall.Wait();
                        //await readAsyncCall;
                        readAsyncCall.TryGetPropertyValue("Result", out object readAsyncCallResult);
                        next = readAsyncCallResult;
                    }
                    else
                    {
                        next = parameters[0];
                    }

                    try
                    {
                        MetricsSender.Histogram(() => {
                            executor(handler, next);
                        }, metricName: "redis_call", additionalTags: new List<string> { $"channel:{channelProp.ToString()}" });
                    }
                    catch (Exception)
                    {
                    }
                }
                catch (Exception ex)
                {
                    if (ex.GetType().Name == "ChannelClosedException")
                    {
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

            return Task.CompletedTask;

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