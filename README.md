# Inject DevOps-things using ICorProfilerCallback

[![Build Status](https://travis-ci.org/ABaboshin/DotNetCoreProfiler.svg?branch=master)](https://travis-ci.org/ABaboshin/DotNetCoreProfiler)

This project has a goal to demonstrate injection of the following devops-related things into .net core 3.1 apps:
 - Logger configuration
 - Distributed tracing for
    - incoming http requests
    - outgoing http requests
    - publish masstransit messages
    - consume masstransit messages
 - Collect time execution metrics for:
    - incoming http requests
    - outgoing http requests
    - masstransit publish
    - masstransit consume
    - EntityFrameworkCore queries
    - Quartz scheduled jobs
    - User-defined using `MonitorAttribute` (see https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/src/Interception.Attributes/MonitorAttribute.cs)
 - Cache using `CacheAttribute`
 - Store collected metrics in:
    - prometheus
    - jaeger
 - Intercept creation of `IServiceProvider` and provide full access to the DI for the injected code
 - Validate method parameters, see i.e. https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/src/Interception.Attributes/Validation/NotNullAttribute.cs
 - Interceptors can be combined. See https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/sample/MessageBus/MyMessageConsumer.cs#L54
 - The order of interecepted is configuratble via `Priority` property.

***Limitations***
 - Dynamic methods are not supported.
 - If you want to use parameter validator, use the `ValidationAttribute` on the method.
 - In some cases to intercept the generic method you have to implement `IMethodFinder` (see https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/src/Interception.MassTransit/ConsumeMethodFinder.cs#L13)

## Project structure

### ICorProfilerCallback implementation

See [src/profiler](src/profiler).

### Interceptors

See [samples/Interception](samples/Interception):
  - `ConfigureServicesBuilderInterceptor` intercept the `Startup.ConfigureServices` call and injects i.e. logger configuration, general tracing configuration, observers
  - `ConfigureBuilderInterceptor` intercepts the `Startup.Configure` call and inject tracing middleware
  - `CreateUsingRabbitMqInterceptor` intercepts the rabbitmq masstransit bus configuration and injects tracing into publishing pipeline
  - `MassTransitConsumerInterceptor` intercepts consuming of masstransit messages in order to handle the tracing
  - `QuartzJobExecuteInterceptor` intercepts the execution of the quartz jobs
  - `MonitoringInterceptor` finds the usage of `MonitorAttribute` and intercepts the marked calls with measuring execution time, In addition the following monitor parameters can be injected:
      - Customited metric name
      - Passed parameters
      - Return value
      - For usage see https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/sample/MessageBus/MyMessageConsumer.cs#L64
  - `CacheInterceptor` finsd the usage of `CacheAttribute` and cache the results for the given amount of seconds, the parameters which have to be taken into accout can be configured:
      - For usage see https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/sample/MessageBus/MyMessageConsumer.cs#L64
  - `InvalidateCacheInterceptor` finds the usage  of `InvalidateCacheAttribute` and invalidates the cache:
      - For usage see https://github.com/ABaboshin/DotNetCoreProfiler/blob/master/sample/MessageBus/MyMessageConsumer.cs#L51

### Observers

See [src/Interception.Observers](src/Interception.Observers):
  - `EntityFrameworkCoreObserver` observers and measure execution time of entity framework core queries
  - `HttpHandlerDiagnostrics` observers and measure execution time of outgoing http requests

### Method parameter validation

See [src/Interception.Attributes](src/Interception.Attributes):
  - `NotNullAttribute` checks if a passed parameter value is not null,
  - `GreatThenZeroAttribute` checks if a passed int value is positive

### Prometheus/Statsd reporter

See [src/Interception.OpenTracing.Prometheus](Interception.OpenTracing.Prometheus) is an implementation of OpenTracing to report the metrics into statsd.

### Sample

See [samples](samples).

In order to run the sample execute

```
# Supply metrics to jaeger, see X-Trace-Id header, Jaeger is hosted under http://localhost:16686/
yarn run:sample:jaeger

# Supply metrics to prometheus, see X-Trace-Id header, prometheus is hosted under http://localhost:9090/, metric name is interception, trace field is X_Trace_Id
yarn run:sample:prometheus

# Supply metrics to prometheus using protobuf, see X-Trace-Id header, prometheus is hosted under http://localhost:9090/, metric name is interception, trace field is X_Trace_Id
# Pull https://github.com/ABaboshin/statsd_exporter
# and build an image docker build -t statsd:protobuf .
yarn run:sample:prometheus:protobuf
```

It will create an app running on `http://localhost:5000` with the following endpoints:
 - api/values/bad -> produces one incoming http metric with an error and one entity framework core metric with an error
![bad query sample](./bad.png)
 - api/values/publish -> produces
    - two incoming http metric
    - one masstransit publish metric
    - one masstransit consume metric
    - one masstransit outgoing http metric
    - three entity framework core metric
    - one custom "A long time action metric" with result and one of the parameters
    - two custom "Fibonacci call" metrics
![good query sample](./good.png)
