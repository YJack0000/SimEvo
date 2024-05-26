find_package(Boost 1.74.0 REQUIRED)

if(Boost_FOUND)
  message(STATUS "Boost found.")
  message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
  message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
  message(
    FATAL_ERROR
      "Boost not found. Please make sure Boost is installed and uuid module is available."
  )
endif()

add_library(
  core
  core/Environment.cpp
  core/Genes.cpp core/Organism.cpp)

add_library(
  index index/DefaultSpatialIndex.cpp index/OptimizedSpatialIndex.cpp
)

target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(index PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(index PUBLIC ${Boost_INCLUDE_DIRS})
