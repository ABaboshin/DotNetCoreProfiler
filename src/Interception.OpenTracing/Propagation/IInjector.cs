using OpenTracing;

namespace Interception.OpenTracing.Propagation
{
    interface IInjector<T>
    {
        void Inject(ISpanContext spanContext, T carrier);
    }
}
