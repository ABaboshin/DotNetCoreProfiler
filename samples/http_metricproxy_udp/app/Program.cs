using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Hosting;
using System.IO;
using System.Reflection;

namespace app
{
  class Program
  {
    public static void Main(string[] args)
    {
      var host = Microsoft.Extensions.Hosting.Host.CreateDefaultBuilder(args)
      .ConfigureWebHostDefaults(webBuilder =>
      {
        webBuilder
          .UseContentRoot(Directory.GetCurrentDirectory())
          .UseKestrel()
          .UseStartup<Startup>();
      })
           .Build();

      host.Run();
    }
  }
}
