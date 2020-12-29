using OpenTracing;

namespace Interception.OpenTracing.Propagation
{
    interface IExtractor<T>
    {
        ISpanContext Extract(T carrier);
    }
}
