cmake_minimum_required (VERSION 3.8)
cmake_policy(SET CMP0015 NEW)

project("DotNetCoreProfiler" VERSION 1.13.3)

add_compile_options(-std=c++11 -fPIC -fms-extensions)
add_compile_options(-DBIT64 -DPAL_STDCPP_COMPAT -DPLATFORM_UNIX -DUNICODE)
add_compile_options(-Wno-invalid-noreturn -Wno-macro-redefined)

SET_SOURCE_FILES_PROPERTIES(
  ${GENERATED_OBJ_FILES}
  PROPERTIES
  EXTERNAL_OBJECT false
  GENERATED true
)

add_library("DotNetCoreProfiler.static" STATIC
    ClassFactory.cpp
    CorProfiler.cpp
    helpers.cpp
    miniutf.cpp
    util.cpp
    miniutf.cpp
    MethodSignature.cpp
    ${GENERATED_OBJ_FILES}
)
set_target_properties("DotNetCoreProfiler.static" PROPERTIES PREFIX "")
target_include_directories("DotNetCoreProfiler.static"
    PUBLIC ../../lib/coreclr/src/pal/inc/rt
    PUBLIC ../../lib/coreclr/src/pal/prebuilt/inc
    PUBLIC ../../lib/coreclr/src/pal/inc
    PUBLIC ../../lib/coreclr/src/inc
)
add_library("DotNetCoreProfiler" SHARED
    dllmain.cpp
)
set_target_properties("DotNetCoreProfiler" PROPERTIES PREFIX "")
target_link_libraries("DotNetCoreProfiler" "DotNetCoreProfiler.static")
