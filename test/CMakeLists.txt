cmake_minimum_required(VERSION 3.24)
project(SimEvoTests)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows : Prevent overriding the parent project's compiler/linker settings
set(GTEST_FORCE_SHARED_CRT ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SPATIAL_INDEX_SOURCES
  ${PROJECT_SOURCE_DIR}/../src/index/DefaultSpatialIndex.cpp
  ${PROJECT_SOURCE_DIR}/../src/index/OptimizedSpatialIndex.cpp
)

add_executable(test_spatial_index SpatialIndexTest.cpp ${SPATIAL_INDEX_SOURCES})

# include from the parent directory
target_include_directories(test_spatial_index PRIVATE ${PROJECT_SOURCE_DIR}/../include)

target_link_libraries(test_spatial_index gtest_main gtest)

add_test(NAME SpatialIndexTest COMMAND test_spatial_index)
set_tests_properties(SpatialIndexTest PROPERTIES LABELS "SpatialIndex")
