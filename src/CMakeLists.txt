add_library(core
    # core/Environment.cpp
    core/Genes.cpp
    core/Organism.cpp
)

add_library(index
    index/DefaultSpatialIndex.cpp
    index/OptimizedSpatialIndex.cpp
)

target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(index PUBLIC ${CMAKE_SOURCE_DIR}/include)
