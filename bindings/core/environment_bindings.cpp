#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>

namespace py = pybind11;

PYBIND11_MODULE(your_module_name, m) {
    py::class_<Organism, std::shared_ptr<Organism>>(m, "Organism")
        .def(py::init<>());

    py::class_<Food, std::shared_ptr<Food>>(m, "Food")
        .def(py::init<>());

    py::class_<Environment, std::shared_ptr<Environment>>(m, "Environment")
        .def(py::init<int, int, const std::string &>())
        .def("addOrganism", &Environment::addOrganism)
        .def("addFood", &Environment::addFood);
}
