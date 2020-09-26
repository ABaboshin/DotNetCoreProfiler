cmake_minimum_required (VERSION 3.17)
cmake_policy(SET CMP0015 NEW)

project("Profiler.Native.Tests" VERSION 1.0.0)

include(EnableGoogleTest.cmake)
include(GoogleTest)

add_compile_options(-std=c++11 -fPIC -fms-extensions -stdlib=libstdc++)
add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE)
add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined -Wno-pragma-pack)

set(GTEST_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/libgtest.a")
find_library(pthreads NAMES libpthread.so FIND_LIBRARY_USE_LIB64_PATHS)

SET_SOURCE_FILES_PROPERTIES(
  ${GENERATED_OBJ_FILES}
  PROPERTIES
  EXTERNAL_OBJECT false
  GENERATED true
)

add_executable(ProfilerTests
    ../../src/profiler/configuration/Configuration.cpp
    ../../src/profiler/info/AssemblyInfo.cpp
    ../../src/profiler/info/ModuleInfo.cpp
    ../../src/profiler/info/parser.cpp
    ../../src/profiler/miniutf/miniutf.cpp
    ../../src/profiler/util/util.cpp
    AssemblyInfoTests.cpp
    ConfigurationTests.cpp
    ModuleInfoTests.cpp
    ParserTests.cpp
    main.cpp
    mocks.cpp
    ${GENERATED_OBJ_FILES}
)

target_include_directories(ProfilerTests
    PUBLIC ../../lib/coreclr/src/pal/inc/rt
    PUBLIC ../../lib/coreclr/src/pal/prebuilt/inc
    PUBLIC ../../lib/coreclr/src/pal/inc
    PUBLIC ../../lib/coreclr/src/inc
    PUBLIC ../../src/profiler
    PUBLIC googletest-src/googletest/include/
    PUBLIC .
)

target_link_libraries(ProfilerTests ${GTEST_LIBRARY} ${pthreads})

gtest_add_tests(TARGET      ProfilerTests
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests
)

