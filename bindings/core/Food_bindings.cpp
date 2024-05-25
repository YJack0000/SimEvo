#include <pybind11/pybind11.h>

#include <core/Food.hpp>

namespace py = pybind11;

void init_Food(py::module &m) {
    py::class_<Food, BaseEnvironmentObject>(m, "Food")
        .def(py::init<>())
        .def("can_be_eaten", &Food::canBeEaten)
        .def("eaten", &Food::eaten);
}