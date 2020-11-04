# Samples

## Deadlock Detection

See [deadlock-detection](deadlock-detection).

In order to run:
```
cd .. && yarn samples:dd
```

This sample demonstrates the detection of deadlocks.

As a result you'll see `Interception.DeadlockDetection.DeadlockException: Monitor o1->Thread  4; Thread  5->Thread  4; Monitor o2->Thread  4` in the output.

## Cache

See [cache](cache).

In order to run:
```
cd .. && yarn samples:cache
```

This sample calculates a fibonacci number and caches the result in redis.

Call `http://localhost:5000/api/sample` multiple time and compare execution time. It will be decreased.
Morevoer you can go inside the `cache_redis_1` container and check the list of keys:

```
docker exec -it cache_redis_1 bash
redis-cli
keys *
# 1) "Fibonacci30-30"
```

To invalidate the cache call `http://localhost:5000/api/sample/invalidate` and check that no key in redis is presenting.

Morover this sample demonstrates:
 - howto inject dependencies from an interceptor,
 - howto use  dependencies from an interceptor,
 - howto write an interceptor for an attribute.

## Rabbitmq (masstransit)

See [rabbitmq](rabbitmq).

In order to run:
```
cd .. && yarn samples:rabbitmq
```

This samples sends and processes the messages with rabbitmq using masstransit:
 - Call `http://localhost:5000/api/sample/bad` to produce one messages which will be consumed with an error
 - Call `http://localhost:5000/api/sample/good` to produce one messages which will be consumed sucessfully

Open `http://localhost:16686/search` to see the metrics.

Moreover this sample demonstrates:
 - howto inject customer `ILogger` from an interceptor,
 - howto report the metrics to jaeger,
 - howto inject custom trace id into logs,
 - howto write an interceptor for an attribute.

## Http (incoming/outgoing)

See [http](http).

In order to run:
```
cd .. && yarn samples:http
```

This samples sends one outgoing http request and receives one incoming http request when calling `http://localhost:5000/api/sample`.

Open `http://localhost:16686/search` to see the metrics.

Moreover this sample demonstrates:
 - howto inject custom trace id into logs,
 - howto inject customer custom trace id into http responses (see `x-trace-id`).

 ## Quartz

See [quartz](quartz).

In order to run:
```
cd .. && yarn samples:quartz
```

This samples schedules one job with quartz.

Open `http://localhost:16686/search` to see the metrics.

Moreover this sample demonstrates:
 - howto inject custom trace id into logs,
 - howto inject customer custom trace id into http responses (see `x-trace-id`).
