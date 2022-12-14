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

add_library("DotNetCoreProfiler.static" STATIC
  profiler/CorProfiler.cpp
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
