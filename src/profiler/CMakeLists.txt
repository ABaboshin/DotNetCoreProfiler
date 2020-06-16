cmake_minimum_required (VERSION 3.17)
cmake_policy(SET CMP0015 NEW)

project("DotNetCoreProfiler" VERSION 1.0.0)

add_compile_options(-std=c++11 -fPIC -fms-extensions -stdlib=libstdc++)
add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE -DLOGGING)
add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined -Wno-pragma-pack)

SET_SOURCE_FILES_PROPERTIES(
  ${GENERATED_OBJ_FILES}
  PROPERTIES
  EXTERNAL_OBJECT false
  GENERATED true
)

add_library("DotNetCoreProfiler.static" STATIC
    ClassFactory.cpp
    logging/JsonFormatter.cpp
    logging/util.cpp
    profiler/CorProfiler.cpp
    miniutf/miniutf.cpp
    configuration/Configuration.cpp
    util/util.cpp
    util/helpers.cpp
    info/GenericMethodSignature.cpp
    info/MethodSignature.cpp
    info/AssemblyInfo.cpp
    info/FunctionInfo.cpp
    info/ModuleInfo.cpp
    info/TypeInfo.cpp
    info/parser.cpp
    rewriter/ILRewriter.cpp
    rewriter/ILRewriterHelper.cpp
    ${GENERATED_OBJ_FILES}
)

set_target_properties("DotNetCoreProfiler.static" PROPERTIES PREFIX "")
target_include_directories("DotNetCoreProfiler.static"
    PUBLIC ../../lib/coreclr/src/pal/inc/rt
    PUBLIC ../../lib/coreclr/src/pal/prebuilt/inc
    PUBLIC ../../lib/coreclr/src/pal/inc
    PUBLIC ../../lib/coreclr/src/inc
    PUBLIC .
)

add_library("DotNetCoreProfiler" SHARED
    dllmain.cpp
)

set_target_properties("DotNetCoreProfiler" PROPERTIES PREFIX "")
target_link_libraries("DotNetCoreProfiler" "DotNetCoreProfiler.static")
