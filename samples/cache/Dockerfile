FROM mcr.microsoft.com/dotnet/sdk:6.0 as build

COPY src src

RUN dotnet publish -c Release -o /generator src/Interception.Generator/Interception.Generator.csproj

COPY samples samples

RUN dotnet publish -c Release -o /app samples/cache/app/app.csproj
RUN dotnet publish -c Release -o /interceptor samples/cache/interceptor/interceptor.csproj

WORKDIR /interceptor

RUN dotnet ../generator/Interception.Generator.dll -a interceptor.dll,Interception.Core.dll,Interception.Cache.dll,Interception.AspNetCore.WebHost.dll -p /profiler -o profiler.json -s /samples/cache/skip.txt

FROM profiler:native as profiler

FROM mcr.microsoft.com/dotnet/aspnet:6.0
RUN apt update && apt install -y dos2unix

EXPOSE 80

WORKDIR /app

ENV CORECLR_ENABLE_PROFILING=1
ENV CORECLR_PROFILER="{585022b6-31e9-4ddf-b35d-3c256d0a16f3}"
ENV CORECLR_PROFILER_PATH="/profiler/DotNetCoreProfiler.so"
ENV PROFILER_CONFIGURATION="/profiler/profiler.json"
ENV PROFILER_LOG_LEVEL="INFO"

ENTRYPOINT ["/app/run.sh"]

RUN mkdir /profiler
COPY --from=profiler /DotNetCoreProfiler.so /profiler

COPY --from=build /app /app
COPY --from=build /samples/cache/run.sh /app
COPY --from=build /interceptor/*.dll /profiler/
COPY --from=build /interceptor/profiler.json /profiler/
RUN dos2unix /app/run.sh && chmod +x /app/run.sh
