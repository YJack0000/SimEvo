#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <core/Environment.hpp>

namespace py = pybind11;

void init_Environment(py::module& m) {
    py::class_<Environment>(m, "Environment")
        .def(py::init<int, int, std::string>(), py::arg("width"),
             py::arg("height"), py::arg("type") = "default")
        .def("get_width", &Environment::getWidth)
        .def("get_height", &Environment::getHeight)
        .def("get_all_objects", &Environment::getAllObjects,
             "Returns a reference to the internal objects map.")
        .def(py::init<int, int, std::string>())
        .def("add_organism", &Environment::addOrganism)
        .def("add_food", &Environment::addFood)
        .def(
            "simulate_iteration",
            [](Environment& env, int iterations,
               py::function on_each_iteration) {
                env.simulateIteration(
                    iterations, [on_each_iteration](const Environment& env) {
                        on_each_iteration(env);
                    });
            },
            py::arg("iterations"), py::arg("on_each_iteration") = py::none())
        .def("post_iteration", &Environment::postIteration);
}
