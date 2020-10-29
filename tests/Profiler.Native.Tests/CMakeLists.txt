cmake_minimum_required (VERSION 3.17)
cmake_policy(SET CMP0015 NEW)

project("Profiler.Native.Tests" VERSION 1.0.0)

include(EnableGoogleTest.cmake)
include(GoogleTest)

IF (WIN32)
  add_definitions(-DUNICODE -D_UNICODE -DWIN32 -DNDEBUG -D_WINDOWS -D_USRDLL)
ENDIF()

IF (UNIX AND NOT APPLE)
  add_compile_options(-std=c++11 -fPIC -fms-extensions -stdlib=libstdc++)
  add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE)
  add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined -Wno-pragma-pack)
ENDIF()

IF (APPLE)
  add_compile_options(-std=c++11 -fPIC -fms-extensions -stdlib=libc++)
  add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE)
  add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined -Wno-pragma-pack)
ENDIF()

SET_SOURCE_FILES_PROPERTIES(
  ${GENERATED_OBJ_FILES}
  PROPERTIES
  EXTERNAL_OBJECT false
  GENERATED true
)

set(src
  ../../src/profiler/configuration/Configuration.cpp
  ../../src/profiler/info/AssemblyInfo.cpp
  ../../src/profiler/info/FunctionInfo.cpp
  ../../src/profiler/info/GenericMethodSignature.cpp
  ../../src/profiler/info/MethodSignature.cpp
  ../../src/profiler/info/ModuleInfo.cpp
  ../../src/profiler/info/TypeInfo.cpp
  ../../src/profiler/info/parser.cpp
  ../../src/profiler/logging/logging.cpp
  ../../src/profiler/miniutf/miniutf.cpp
  ../../src/profiler/profiler/CorProfiler.cpp
  ../../src/profiler/rewriter/ILRewriter.cpp
  ../../src/profiler/rewriter/ILRewriterHelper.cpp
  ../../src/profiler/util/helpers.cpp
  ../../src/profiler/util/util.cpp
  AssemblyInfoTests.cpp
  ConfigurationTests.cpp
  CorProfilerTests.cpp
  ModuleInfoTests.cpp
  ParserTests.cpp
  main.cpp
  ${GENERATED_OBJ_FILES}
)

IF (UNIX)
  set(src
    mocks.cpp
    ${src}
  )
ENDIF()

add_executable(ProfilerTests
  ${src}
)

target_include_directories(ProfilerTests
  PUBLIC ../../lib/coreclr/src/inc
  PUBLIC ../../src/profiler
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googletest/include/
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googlemock/include/
  PUBLIC .
)

IF (UNIX)
  target_include_directories(ProfilerTests
    PUBLIC ../../lib/coreclr/src/pal/inc/rt
    PUBLIC ../../lib/coreclr/src/pal/prebuilt/inc
    PUBLIC ../../lib/coreclr/src/pal/inc
    PUBLIC ../../lib/coreclr/src/inc
  )
ENDIF()

target_link_libraries(ProfilerTests gtest_main gmock_main)

gtest_add_tests(TARGET      ProfilerTests
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests
)
