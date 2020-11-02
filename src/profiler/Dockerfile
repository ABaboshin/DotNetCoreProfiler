FROM ababoshin/dotnetcoreprofiler:llvm-10 as build

WORKDIR /profiler
COPY lib /profiler/lib
COPY src/profiler /profiler/src/profiler

WORKDIR /profiler/src/profiler

RUN cmake -DCMAKE_BUILD_TYPE=Release . && make && cp DotNetCoreProfiler.so /

FROM scratch

COPY --from=build /DotNetCoreProfiler.so /
