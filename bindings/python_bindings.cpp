#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_BaseEnvironmentObject(py::module &);
void init_Environment(py::module &);
void init_Food(py::module &);
void init_Genes(py::module &);
void init_Organism(py::module &);

PYBIND11_MODULE(simevopy, m) {
    init_BaseEnvironmentObject(m);
    init_Environment(m);
    init_Food(m);
    init_Genes(m);
    init_Organism(m);
}
