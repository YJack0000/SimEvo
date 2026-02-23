cmake_minimum_required(VERSION 3.24)
project(SimEvoTests)

include(gtest.cmake)

# add the test executable
add_executable(test_spatial_index_uuid SpatialIndexUUIDTest.cpp)

# include from the parent directory
target_include_directories(test_spatial_index_uuid
                           PRIVATE ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(test_spatial_index_uuid index gtest_main gtest)

# add the test to the test suite
add_test(NAME SpatialIndexUUIDTest COMMAND test_spatial_index_uuid)
set_tests_properties(SpatialIndexUUIDTest PROPERTIES LABELS "SpatialIndex")

# benchmark executable
add_executable(benchmark_spatial_index SpatialIndexBenchmark.cpp)
target_include_directories(benchmark_spatial_index
                           PRIVATE ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(benchmark_spatial_index index gtest_main gtest)
add_test(NAME SpatialIndexBenchmark COMMAND benchmark_spatial_index)
set_tests_properties(SpatialIndexBenchmark PROPERTIES LABELS "Benchmark")
