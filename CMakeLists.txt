cmake_minimum_required(VERSION 3.24)
project(SimEvo)

# check if the compiler supports C++20
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)

if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} does not support C++20.")
endif()

# include the headers in the include directory
include_directories(${PROJECT_SOURCE_DIR} / include)

option(BUILD_TESTS "Build the test programs" ON)
# add_subdirectory(src)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
