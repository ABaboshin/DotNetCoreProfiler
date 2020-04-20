using OpenTracing;

namespace Interception.OpenTracing.Prometheus.Propagation
{
    interface IInjector<T>
    {
        void Inject(ISpanContext spanContext, T carrier);
    }
}
