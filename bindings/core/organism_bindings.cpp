#include <pybind11/pybind11.h>
#include <core/Organism.hpp>

namespace py = pybind11;

void init_Organism(py::module &m) {
    py::class_<Organism, BaseEnvironmentObject, std::shared_ptr<Organism>>(m, "Organism")
        .def(py::init<const Genes &>())
        .def(py::init<const Genes &, Organism::LifeConsumptionCalculator>())
        .def("get_speed", &Organism::getSpeed)
        .def("get_size", &Organism::getSize)
        .def("get_awareness", &Organism::getAwareness)
        .def("get_life_consumption", &Organism::getLifeConsumption)
        .def("killed", &Organism::killed)
        .def("is_alive", &Organism::isAlive)
        .def("get_reaction_radius", &Organism::getReactionRadius)
        .def("interact", &Organism::interact)
        .def("post_iteration", &Organism::postIteration);
}
