using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.EntityFrameworkCore;
using System;
using System.Linq;
using app.Database.Entities;
using app.Database;

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
            ConfigureDatabase(serviceCollection);
            serviceCollection
                .AddMvc(options => { options.EnableEndpointRouting = false; });
        }

        private void ConfigureDatabase(IServiceCollection services)
        {
            services.Configure<DatabaseConfiguration>(Configuration.GetSection(DatabaseConfiguration.SectionKey));
            services.AddDbContext<MyDbContext>(options => options.UseSqlServer(Configuration.GetSection(DatabaseConfiguration.SectionKey).Get<DatabaseConfiguration>().ConnectionString));
        }

        public void Configure(IApplicationBuilder app)
        {
            app.UseMvc();
            MigrateDatabase(app);
        }

        private static void MigrateDatabase(IApplicationBuilder app)
        {
            using (var scope = app.ApplicationServices.GetService<IServiceScopeFactory>().CreateScope())
            {
                scope.ServiceProvider.GetRequiredService<MyDbContext>().Database.Migrate();

                var context = scope.ServiceProvider.GetRequiredService<MyDbContext>();
                if (!context.MyEntities.Any())
                {
                    context.MyEntities.Add(new MyEntity { Value = "1" });
                    context.MyEntities.Add(new MyEntity { Value = "2" });
                    context.MyEntities.Add(new MyEntity { Value = "3" });
                    context.SaveChanges();
                }
            }
        }
    }
}
