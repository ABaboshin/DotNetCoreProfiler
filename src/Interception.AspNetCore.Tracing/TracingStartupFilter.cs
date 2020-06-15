using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using System;

namespace Interception.AspNetCore
{
    public class TracingStartupFilter : IStartupFilter
    {
        public Action<IApplicationBuilder> Configure(Action<IApplicationBuilder> next)
        {
            return applicationBuilder =>
            {
                applicationBuilder.UseMiddleware<TracingMiddleware>();
                next(applicationBuilder);
            };
        }
    }
}
