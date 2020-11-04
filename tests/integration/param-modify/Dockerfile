FROM mcr.microsoft.com/dotnet/core/sdk:3.1.301 as build

COPY src src
COPY tests tests

RUN dotnet publish -c Release -o /generator src/Interception.Generator/Interception.Generator.csproj
RUN dotnet publish -c Release -o /app tests/integration/param-modify/app/app.csproj
RUN dotnet publish -c Release -o /interceptor tests/integration/param-modify/interceptor/interceptor.csproj

WORKDIR /interceptor

RUN dotnet ../generator/Interception.Generator.dll -a interceptor.dll,Interception.Core.dll -p /profiler -o profiler.json -s /tests/integration/skip.txt

FROM profiler:native as profiler

FROM mcr.microsoft.com/dotnet/core/aspnet:3.1.5-bionic
RUN apt update && apt install -y dos2unix

EXPOSE 80

WORKDIR /app

ENV CORECLR_ENABLE_PROFILING=1
ENV CORECLR_PROFILER="{585022b6-31e9-4ddf-b35d-3c256d0a16f3}"
ENV CORECLR_PROFILER_PATH="/profiler/DotNetCoreProfiler.so"
ENV PROFILER_CONFIGURATION="/profiler/profiler.json"
ENV PROFILER_LOG_LEVEL="NONE"

ENTRYPOINT ["/app/run.sh"]

RUN mkdir /profiler
COPY --from=profiler /DotNetCoreProfiler.so /profiler

COPY --from=build /app /app
COPY --from=build /tests/integration/param-modify/expected.txt /app
COPY --from=build /tests/integration/param-modify/run.sh /app
RUN dos2unix /app/run.sh && chmod +x /app/run.sh
COPY --from=build /interceptor/*.dll /profiler/
COPY --from=build /interceptor/profiler.json /profiler/
