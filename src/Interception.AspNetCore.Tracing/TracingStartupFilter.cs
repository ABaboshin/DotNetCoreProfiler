using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using System;

namespace Interception.AspNetCore
{
    public class TracingStartupFilter : IStartupFilter
    {
        public Action<IApplicationBuilder> Additional { get; set; }

        public Action<IApplicationBuilder> Configure(Action<IApplicationBuilder> next)
        {
            return applicationBuilder =>
            {
                if (Additional != null)
                {
                    Additional(applicationBuilder);
                }
                applicationBuilder.UseMiddleware<TracingMiddleware>();
                next(applicationBuilder);
            };
        }
    }
}
