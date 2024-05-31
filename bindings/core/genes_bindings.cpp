#include <pybind11/pybind11.h>

#include <core/Genes.hpp>

namespace py = pybind11;

void init_Genes(py::module &m) {
    py::class_<Genes>(m, "Genes")
        .def(py::init<const char *>())
        .def(py::init<const char *, Genes::MutationFunction>())
        .def("mutate", &Genes::mutate)
        .def("get_dna", &Genes::getDNA);
}
