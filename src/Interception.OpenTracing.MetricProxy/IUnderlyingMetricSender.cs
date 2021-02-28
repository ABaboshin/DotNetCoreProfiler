namespace Interception.OpenTracing.MetricProxy
{
    internal interface IUnderlyingMetricSender
    {
        bool Send(TraceMetric metric);
    }
}
