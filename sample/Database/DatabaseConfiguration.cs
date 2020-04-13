using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace SampleApp.Database
{
    public class DatabaseConfiguration
    {
        public const string SectionKey = "DATABASE";
        public string ConnectionString { get; set; }
    }
}
