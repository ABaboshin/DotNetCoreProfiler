namespace Interception.OpenTracing
{
  public interface IMetricsSender
    {
        void Histogram(Span span);
    }
}
