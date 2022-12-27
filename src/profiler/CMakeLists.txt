cmake_minimum_required (VERSION 3.17)
cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0091 NEW)

project("DotNetCoreProfiler" VERSION 1.6.0)

IF (WIN32)
  add_definitions(-DUNICODE -D_UNICODE -DWIN32 -DNDEBUG -D_WINDOWS -D_USRDLL -D_WIN32 -DWINVER=0x0602 -D_WIN32_WINNT=0x0602 -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
ENDIF()

IF (UNIX AND NOT APPLE)
  add_compile_options(-std=c++11 -fPIC -fms-extensions -stdlib=libstdc++)
  add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE)
  add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined -Wno-pragma-pack -g -Wall -Wno-null-conversion)
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

# include(FetchContent)

# FetchContent_Declare(fmt
#   GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#   GIT_TAG master
# )

# FetchContent_MakeAvailable(fmt)

# # add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/_deps/fmt-src/include
# #                  EXCLUDE_FROM_ALL)

add_library("DotNetCoreProfiler.static" STATIC
  profiler/CorProfiler.cpp
  profiler/CorProfiler.ModuleLoadFinished.cpp
  profiler/CorProfiler.JITCompilationStarted.cpp
  profiler/CorProfiler.Rejit.cpp
  profiler/MethodRewriter.cpp
  profiler/MethodRewriter.LocalVariables.cpp
  profiler/MethodRewriter.CreateAfterMethod.cpp
  profiler/MethodRewriter.CreateBeforeMethod.cpp
  profiler/MethodRewriter.LogInterceptorException.cpp
  ClassFactory.cpp
  configuration/Configuration.cpp
  info/AssemblyInfo.cpp
  info/FunctionInfo.cpp
  info/GenericMethodSignature.cpp
  info/MethodSignature.cpp
  info/ModuleInfo.cpp
  info/TypeInfo.cpp
  info/parser.cpp
  logging/logging.cpp
  miniutf/miniutf.cpp
  rewriter/ILRewriter.cpp
  rewriter/ILRewriterHelper.cpp
  util/helpers.cpp
  util/util.cpp
  ${GENERATED_OBJ_FILES}
)

set_target_properties("DotNetCoreProfiler.static" PROPERTIES PREFIX "")
target_include_directories("DotNetCoreProfiler.static"
  PUBLIC ../../lib/coreclr/src/inc
  PUBLIC .
  # PuBLIC include
)

IF (WIN32)
ELSE()
  target_include_directories("DotNetCoreProfiler.static"
    PUBLIC ../../lib/coreclr/src/pal/inc/rt
    PUBLIC ../../lib/coreclr/src/pal/prebuilt/inc
    PUBLIC ../../lib/coreclr/src/pal/inc
  )
ENDIF()

IF (WIN32)
  add_library("DotNetCoreProfiler" SHARED
    profiler.def
    dllmain.cpp
  )
ELSE()
  add_library("DotNetCoreProfiler" SHARED
    dllmain.cpp
  )
ENDIF()

set_target_properties("DotNetCoreProfiler" PROPERTIES PREFIX "")
target_link_libraries("DotNetCoreProfiler" "DotNetCoreProfiler.static")
