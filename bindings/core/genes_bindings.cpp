#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <core/Genes.hpp>

namespace py = pybind11;

void define_genes(py::module &m) {
    py::class_<Genes>(m, "Genes")
        .def(py::init<const char*, Genes::MutationFunction>(),
             py::arg("dna_str"), py::arg("custom_mutation_logic") = nullptr)
        .def("get_gene", &Genes::getGene);
}
