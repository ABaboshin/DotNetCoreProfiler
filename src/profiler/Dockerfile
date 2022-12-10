FROM profiler:llvm as build

COPY lib /profiler/lib
COPY src/profiler /profiler/src/profiler

WORKDIR /profiler/src/profiler

RUN find . -iwholename '*cmake*' -not -name CMakeLists.txt -delete
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make && cp DotNetCoreProfiler.so /

FROM scratch

COPY --from=build /DotNetCoreProfiler.so /
