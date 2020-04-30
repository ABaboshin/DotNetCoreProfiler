using Microsoft.AspNetCore.Builder;

namespace Interception.AspNetCore
{
    public static class TracingMiddlewareExtensions
    {
        public static IApplicationBuilder UseTracingMiddleware(this IApplicationBuilder app)
        {
            return app.UseMiddleware<TracingMiddleware>();
        }
    }
}
