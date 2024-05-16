cmake_minimum_required(VERSION 3.24)
project(SimEvoTests)

include(gtest.cmake)

# add the test executable
add_executable(test_spatial_index SpatialIndexTest.cpp)

# include from the parent directory
target_include_directories(test_spatial_index PRIVATE ${PROJECT_SOURCE_DIR}/../include)

target_link_libraries(test_spatial_index index gtest_main gtest)

# add the test to the test suite
add_test(NAME SpatialIndexTest COMMAND test_spatial_index)
set_tests_properties(SpatialIndexTest PROPERTIES LABELS "SpatialIndex")
