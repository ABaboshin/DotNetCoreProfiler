FROM mcr.microsoft.com/dotnet/core/sdk:3.1.301 as build

COPY src src

RUN dotnet publish -c Release -o /generator src/Interception.Generator/Interception.Generator.csproj

COPY samples samples

RUN dotnet publish -c Release -o /app samples/http_metricproxy/app/app.csproj
RUN dotnet publish -c Release -o /interceptor samples/http_metricproxy/interceptor/interceptor.csproj

WORKDIR /interceptor

RUN dotnet ../generator/Interception.Generator.dll -a interceptor.dll,Interception.Core.dll,Interception.AspNetCore.WebHost.dll -p /profiler -o profiler.json -s /samples/http/skip.txt

FROM profiler:native as profiler

FROM interception:metricproxy as metricproxy

FROM mcr.microsoft.com/dotnet/core/aspnet:3.1.5-bionic
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
COPY --from=metricproxy /metricproxy /profiler
COPY --from=profiler /DotNetCoreProfiler.so /profiler

COPY --from=build /app /app
COPY --from=build /samples/http_metricproxy/run.sh /app
COPY --from=build /interceptor/*.dll /profiler/
COPY --from=build /interceptor/profiler.json /profiler/
RUN dos2unix /app/run.sh && chmod +x /app/run.sh && chmod +x /profiler/metricproxy
