#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <core/Organism.hpp>

namespace py = pybind11;

void define_organism(py::module &m) {
    py::class_<Organism>(m, "Organism")
        .def(py::init<const Genes&, Organism::LifeConsumptionCalculator>(),
             py::arg("genes"), py::arg("calculator"))
        .def("get_speed", &Organism::getSpeed)
        .def("get_size", &Organism::getSize)
        .def("get_awareness", &Organism::getAwareness)
        .def("get_life_consumption", &Organism::getLifeConsumption);
}
