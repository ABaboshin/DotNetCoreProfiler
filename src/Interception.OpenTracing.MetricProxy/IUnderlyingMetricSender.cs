namespace Interception.OpenTracing.MetricProxy
{
    internal interface IUnderlyingMetricSender
    {
        void Send(TraceMetric metric);
    }
}