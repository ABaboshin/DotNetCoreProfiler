# Collect metrics using ICorProfiler interface

This project demonstrates a way to collect the time execution metrics for .net core 2.1
using ICorProfilerCallback8 interface (https://docs.microsoft.com/en-us/dotnet/framework/unmanaged-api/profiling/icorprofilercallback8-interface).

## Project structure

### CorProfiler implementation

See [src/profiler](src/profiler).

### Interceptors

See [src/Interception](src/Interception):
 - `DbCommandInterception` intercept calls to `DbCommand` such as Execute*. Produces `function_call{name="DbCommand.ExecuteNonQuery"}` metrics.
 - `StackExchangeRedisInterception` intercepts the subscription (https://redis.io/topics/pubsub). Produces `redis_call` metrics.
 - `MassTransitInterception` intercepts calls to `IConsumer<T>.Consume`. Produces `masstransit` metrics.
 - `ConfigureServicesBuilderInterceptor` injects a diagnostic events listener for asp.net core http events.

### Sample

In order to run the sample execute
```
yarn run:all:21
```

It will create an app running on `http://localhost:5000` with the following endpoints:
 - api/values -> produces one http metrics with success and some entity framework core metric with success
 - api/values/bad -> produces one http metrics with an error and one entity framework core metric with an error
 - api/values/publish -> produces one http metrics with success and one masstransit metric with an error
 - api/values/publish-redis -> produces one http metrics with success and one redis metric with success
 - api/values/exception -> produces one http metrics with an error
