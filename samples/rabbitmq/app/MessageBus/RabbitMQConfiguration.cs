using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace app.MessageBus
{
    public class RabbitMQConfiguration
    {
        public static readonly string SectionKey = "rabbitmq";
        public string Host { get; set; }
        public string User { get; set; }
        public string Password { get; set; }
    }
}
