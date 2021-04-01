using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Linq;

namespace app
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        public void ConfigureServices(IServiceCollection serviceCollection)
        {
            serviceCollection
                .AddMvc(options => { options.EnableEndpointRouting = false; });
        }

        public void Configure(IApplicationBuilder app)
        {
            app.UseMvc();
        }
    }
}
