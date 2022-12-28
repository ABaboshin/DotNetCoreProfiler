cmake_minimum_required(VERSION 3.10)
include(ExternalProject)
ExternalProject_Add(fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
  EXCLUDE_FROM_ALL TRUE
  BUILD_COMMAND $(MAKE) fmt
  STEP_TARGETS build)
set(fmtlib_BINARY_DIR "${CMAKE_BINARY_DIR}/fmtlib-prefix/src/fmtlib-build")
set(fmtlib_SOURCE_DIR "${CMAKE_BINARY_DIR}/fmtlib-prefix/src/fmtlib/include")
# add_executable(fmttest fmttest.cpp)
# add_dependencies(fmttest fmtlib-build)
# target_link_libraries(fmttest ${fmtlib_BINARY_DIR}/libfmt.a)
# target_include_directories(fmttest PUBLIC ${fmtlib_SOURCE_DIR})
