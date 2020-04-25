using System.Collections.Generic;

namespace Interception.StackExchangeRedis
{
    public class InterceptionMessage
    {
        public Dictionary<string, string> Tracing { get; set; }
        public string Message { get; set; }
    }
}
