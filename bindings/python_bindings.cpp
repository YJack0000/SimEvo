#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_EnvironmentObject(py::module &);
void init_Environment(py::module &);
void init_Food(py::module &);
void init_Genes(py::module &);
void init_Organism(py::module &);

PYBIND11_MODULE(simevopy, m) {
    m.doc() = "Simulation Evolution Python bindings";
    m.def(
        "hello_world", []() { return "Hello, World!"; },
        "A function that returns a hello world to test the bindings. ");
    init_EnvironmentObject(m);
    init_Environment(m);
    init_Food(m);
    init_Genes(m);
    init_Organism(m);

    py::register_exception<std::out_of_range>(m, "OutOfRangeException");
}
