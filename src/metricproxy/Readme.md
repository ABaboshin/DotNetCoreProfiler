# Metric Proxy

A proxy server for forwarding the tracing metrics from an application to a statsd exporter.

## Settings

 - `METRIC_PROXY_SERVER__UDP` to listen on an udp port, format `:10000`
 - `METRIC_PROXY_SERVER__TCP` to listen on an tcp port, format `:10000`
 - `METRIC_PROXY_SERVER__UDS` to listen on a unix domain socket, format `/var/metricproxy`
 - `METRIC_PROXY_SERVER__STATSD` an address of a statsd server like `statsd:9125`
