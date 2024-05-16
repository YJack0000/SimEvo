#include <pybind11/pybind11.h>

namespace py = pybind11;

void define_genes(py::module &);
void define_organism(py::module &);

PYBIND11_MODULE(simevo, m) {
    define_genes(m);
    define_organism(m);
}
