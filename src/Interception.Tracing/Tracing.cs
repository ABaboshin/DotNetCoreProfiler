using OpenTracing;
using OpenTracing.Util;
using System.Threading;

namespace Interception.Tracing
{
    public class Tracing
    {
        private static AsyncLocal<IScope> _currentScope = new AsyncLocal<IScope>();

        public static IScope CurrentScope
        {
            get => _currentScope.Value;
            set => _currentScope.Value = value;
        }

        public static ITracer Tracer
        {
            get => GlobalTracer.Instance;
        }
    }
}
