FROM profiler:llvm as build

COPY lib /profiler/lib
COPY src/profiler /profiler/src/profiler
COPY tests /profiler/tests

WORKDIR /profiler/tests/Profiler.Native.Tests

RUN cmake -DCMAKE_BUILD_TYPE=Release . && make gtest && make && make test
