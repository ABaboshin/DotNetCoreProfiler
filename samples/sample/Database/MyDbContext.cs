using Microsoft.EntityFrameworkCore;
using SampleApp.Database.Entities;

namespace SampleApp.Database
{
    public class MyDbContext : DbContext
    {
        public MyDbContext(DbContextOptions<MyDbContext> options) : base(options)
        {
        }

        public DbSet<MyEntity> MyEntities { get; set; }
        public DbSet<BadEntity> BadEntities { get; set; }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            modelBuilder.Entity<MyEntity>().ToTable("MyEntity");
        }
    }
}
