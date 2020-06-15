using CommandLine;
using System.Collections.Generic;

namespace Interception.Generator
{
    public class Options
    {
        [Option('a', "assembiles", Separator = ',')]
        public IList<string> Assemblies { get; set; }

        [Option('p', "path")]
        public string Path { get; set; }

        [Option('o', "output")]
        public string Output { get; set; }


        [Option('s', "skip")]
        public string Skip { get; set; }
    }
}
