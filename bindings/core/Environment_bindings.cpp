#include <pybind11/pybind11.h>

#include <core/Environment.hpp>

namespace py = pybind11;

void init_Environment(py::module &m) {
    py::class_<Environment>(m, "Environment")
        .def(py::init<int, int>())
        .def("add_organism", &Environment::addOrganism)
        .def("add_food", &Environment::addFood)
        .def("simulate_iteration", &Environment::simulateIteration)
        .def("post_iteration", &Environment::postIteration);
}
