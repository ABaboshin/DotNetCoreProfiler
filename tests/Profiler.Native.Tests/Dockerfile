FROM ababoshin/dotnetcoreprofiler:llvm-10 as build

WORKDIR /profiler
COPY lib /profiler/lib
COPY src/profiler /profiler/src/profiler
COPY tests /profiler/tests

WORKDIR /profiler/tests/Profiler.Native.Tests

RUN cmake -DCMAKE_BUILD_TYPE=Release . && make gtest && make && make test
