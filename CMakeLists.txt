cmake_minimum_required(VERSION 3.24)
project(SimEvo)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# check if the compiler supports C++20
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)

if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
else()
  message(
    FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} does not support C++20.")
endif()

# include the headers
include_directories(include)

# add the subdirectories
add_subdirectory(src)

# using BUILD_BINDINGS to enable/disable the python bindings
option(BUILD_BINDINGS "Build Python bindings" ON)
if(BUILD_BINDINGS)
  add_subdirectory(bindings)
endif()

# using BUILD_TESTS to enable/disable the tests
option(BUILD_TESTS "Build the test programs" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests/cpp)
endif()
