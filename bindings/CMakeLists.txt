cmake_minimum_required(VERSION 3.24)
project(simevo_bindings)

include(pybind11.cmake)

find_package(Python3 COMPONENTS Development REQUIRED)
# find_package(pybind11 REQUIRED)

pybind11_add_module(simevo
    python_bindings.cpp
    core/genes_bindings.cpp
    core/organism_bindings.cpp
)

target_include_directories(simevo PRIVATE ../include)
target_link_libraries(simevo PRIVATE core index)
