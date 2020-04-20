using OpenTracing;

namespace Interception.OpenTracing.Prometheus.Propagation
{
    interface IExtractor<T>
    {
        ISpanContext Extract(T carrier);
    }
}
