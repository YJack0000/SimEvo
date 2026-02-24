#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include <core/Organism.hpp>

namespace py = pybind11;

void init_Organism(py::module &m) {
    py::class_<Organism, EnvironmentObject, std::shared_ptr<Organism>>(m, "Organism")
        .def(py::init<const Genes &>())
        .def(py::init<const Genes &, Organism::LifeConsumptionCalculator>())
        .def("get_speed", &Organism::getSpeed)
        .def("get_size", &Organism::getSize)
        .def("get_awareness", &Organism::getAwareness)
        .def("get_life_span", &Organism::getLifeSpan)
        .def("get_life_consumption", &Organism::getLifeConsumption)
        .def("killed", &Organism::killed)
        .def("is_alive", &Organism::isAlive)
        .def("can_reproduce", &Organism::canReproduce)
        .def("add_life_span", &Organism::addLifeSpan, py::arg("amount"))
        .def("reproduce", &Organism::reproduce)
        .def("get_reaction_radius", &Organism::getReactionRadius)
        .def("interact", &Organism::interact)
        .def("react", &Organism::react)
        .def("post_iteration", &Organism::postIteration)
        .def("set_reaction_strategy", &Organism::setReactionStrategy, py::arg("strategy"),
             "Set a custom reaction strategy. The callable receives (organism, nearby_objects) "
             "and should return a (dx, dy) tuple for movement direction, or (0, 0) for no reaction.")
        .def("set_interaction_strategy", &Organism::setInteractionStrategy, py::arg("strategy"),
             "Set a custom interaction strategy. The callable receives (organism, nearby_objects) "
             "and should perform interactions (e.g., eat food, kill organisms).");
}
