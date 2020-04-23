using Newtonsoft.Json;
using OpenTracing.Propagation;
using OpenTracing.Tag;
using OpenTracing.Util;
using StackExchange.Redis;
using System;
using System.Collections.Generic;
using System.Net;
using System.Threading.Tasks;

namespace Interception.StackExchangeRedis
{
    public class InterceptionSubscriber : ISubscriber
    {
        private readonly ISubscriber _subscriber;

        public InterceptionSubscriber(ISubscriber subscriber)
        {
            _subscriber = subscriber;
        }

        public IConnectionMultiplexer Multiplexer => _subscriber.Multiplexer;

        public EndPoint IdentifyEndpoint(RedisChannel channel, CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.IdentifyEndpoint(channel, flags);
        }

        public Task<EndPoint> IdentifyEndpointAsync(RedisChannel channel, CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.IdentifyEndpointAsync(channel, flags);
        }

        public bool IsConnected(RedisChannel channel = default)
        {
            return _subscriber.IsConnected(channel);
        }

        public TimeSpan Ping(CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.Ping(flags);
        }

        public Task<TimeSpan> PingAsync(CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.PingAsync(flags);
        }

        public long Publish(RedisChannel channel, RedisValue message, CommandFlags flags = CommandFlags.None)
        {
            Console.WriteLine($"publish {channel} {message}");
            var baseSpan = GlobalTracer.Instance
                .BuildSpan(StackExchangeRedisInterception.RedisConfiguration.PublisherName)
                .AsChildOf(GlobalTracer.Instance.ActiveSpan);
            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindProducer)
                    .SetTag("channel", channel)
                    .SetTag("sync", true);

                var tracing = new Dictionary<string, string>();
                GlobalTracer.Instance.Inject(span.Context, BuiltinFormats.TextMap, new StackExchangeRedisTextMapInjectAdapter(tracing));

                var interceptionMessage = new InterceptionMessage { Tracing = tracing, Message = message };
                var json = JsonConvert.SerializeObject(interceptionMessage);

                Console.WriteLine($"json {json}");

                return _subscriber.Publish(channel, json, flags);
            }
        }

        public async Task<long> PublishAsync(RedisChannel channel, RedisValue message, CommandFlags flags = CommandFlags.None)
        {
            Console.WriteLine($"PublishAsync {channel} {message}");
            var baseSpan = GlobalTracer.Instance
                .BuildSpan(StackExchangeRedisInterception.RedisConfiguration.PublisherName)
                .AsChildOf(GlobalTracer.Instance.ActiveSpan);
            using (var scope = baseSpan.StartActive(finishSpanOnDispose: true))
            {
                var span = scope.Span
                    .SetTag(Tags.SpanKind, Tags.SpanKindProducer)
                    .SetTag("channel", channel)
                    .SetTag("async", true);

                var data = new Dictionary<string, string>();
                GlobalTracer.Instance.Inject(span.Context, BuiltinFormats.TextMap, new StackExchangeRedisTextMapInjectAdapter(data));

                var interceptionMessage = new InterceptionMessage { Tracing = data, Message = message };
                var json = JsonConvert.SerializeObject(interceptionMessage);

                Console.WriteLine($"json {json}");

                return await _subscriber.PublishAsync(channel, json, flags);
            }
        }

        public void Subscribe(RedisChannel channel, Action<RedisChannel, RedisValue> handler, CommandFlags flags = CommandFlags.None)
        {
            _subscriber.Subscribe(channel, handler, flags);
        }

        public ChannelMessageQueue Subscribe(RedisChannel channel, CommandFlags flags = CommandFlags.None)
        {
            Console.WriteLine($"subscribe {channel}");
            return _subscriber.Subscribe(channel, flags);
        }

        public Task SubscribeAsync(RedisChannel channel, Action<RedisChannel, RedisValue> handler, CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.SubscribeAsync(channel, handler, flags);
        }

        public Task<ChannelMessageQueue> SubscribeAsync(RedisChannel channel, CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.SubscribeAsync(channel, flags);
        }

        public EndPoint SubscribedEndpoint(RedisChannel channel)
        {
            return _subscriber.SubscribedEndpoint(channel);
        }

        public bool TryWait(Task task)
        {
            return _subscriber.TryWait(task);
        }

        public void Unsubscribe(RedisChannel channel, Action<RedisChannel, RedisValue> handler = null, CommandFlags flags = CommandFlags.None)
        {
            _subscriber.Unsubscribe(channel, handler, flags);
        }

        public void UnsubscribeAll(CommandFlags flags = CommandFlags.None)
        {
            _subscriber.UnsubscribeAll(flags);
        }

        public Task UnsubscribeAllAsync(CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.UnsubscribeAllAsync(flags);
        }

        public Task UnsubscribeAsync(RedisChannel channel, Action<RedisChannel, RedisValue> handler = null, CommandFlags flags = CommandFlags.None)
        {
            return _subscriber.UnsubscribeAsync(channel, handler, flags);
        }

        public void Wait(Task task)
        {
            _subscriber.Wait(task);
        }

        public T Wait<T>(Task<T> task)
        {
            return _subscriber.Wait(task);
        }

        public void WaitAll(params Task[] tasks)
        {
            _subscriber.WaitAll(tasks);
        }
    }
}
