using OpenTracing;

namespace Interception.OpenTracing
{
    public class Reference
    {
        public ISpanContext Context { get; }
        public string Type { get; }

        public Reference(ISpanContext context, string type)
        {
            Context = context;
            Type = type;
        }
    }
}
