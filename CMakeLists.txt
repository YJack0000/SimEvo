cmake_minimum_required(VERSION 3.24)
project(SimEvo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/include)

option(BUILD_TESTS "Build the test programs" ON)
# add_subdirectory(src)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
