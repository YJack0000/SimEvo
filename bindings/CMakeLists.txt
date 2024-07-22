cmake_minimum_required(VERSION 3.24)
project(simevo_bindings)

find_package(pybind11 CONFIG QUIET)
if(pybind11_FOUND)
  message(STATUS "Using installed pybind11")
else()
  message(STATUS "pybind11 not found, downloading it now")
  include(pybind11.cmake)
endif()

find_package(
  Python3
  COMPONENTS Development
  REQUIRED)

pybind11_add_module(
  simevopy
  python_bindings.cpp
  core/EnvironmentObject_bindings.cpp
  core/Environment_bindings.cpp
  core/Food_bindings.cpp
  core/Genes_bindings.cpp
  core/Organism_bindings.cpp)

target_include_directories(simevopy PUBLIC ../include)
target_link_libraries(simevopy PUBLIC core index)

# set python site-packages
execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# install the python module
install(TARGETS simevopy LIBRARY DESTINATION ${SITE_PACKAGES})
