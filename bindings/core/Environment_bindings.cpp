#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <core/Environment.hpp>
#include <stdexcept>

namespace py = pybind11;

void init_Environment(py::module& m) {
    py::class_<Environment, std::shared_ptr<Environment>>(m, "Environment")
        .def(py::init<int, int, std::string, int>(), py::arg("width"), py::arg("height"),
            py::arg("type") = "default", py::arg("threads") = 1,
             "Constructor for Environment class taking width, height, and an optional type.")
        .def("get_width", &Environment::getWidth, "Get the width of the environment.")
        .def("get_height", &Environment::getHeight, "Get the height of the environment.")
        .def("add_organism",
             static_cast<void (Environment::*)(const std::shared_ptr<Organism>&, float, float)>(
                 &Environment::add),
             py::arg("organism"), py::arg("x"), py::arg("y"),
             "Add an organism at specified coordinates.")
        .def("add_food",
             static_cast<void (Environment::*)(const std::shared_ptr<Food>&, float, float)>(
                 &Environment::add),
             py::arg("food"), py::arg("x"), py::arg("y"), "Add food at specified coordinates.")
        .def("remove_organism",
             static_cast<void (Environment::*)(const std::shared_ptr<Organism>&)>(
                 &Environment::remove),
             py::arg("organism"), "Remove an organism from the environment.")
        .def("remove_food",
             static_cast<void (Environment::*)(const std::shared_ptr<Food>&)>(&Environment::remove),
             py::arg("food"), "Remove food from the environment.")
        .def("reset", &Environment::reset, "Reset the environment.")
        .def("get_all_objects", &Environment::getAllObjects, "Get all objects in the environment.")
        .def("get_all_organisms", &Environment::getAllOrganisms,
             "Get all organisms in the environment.")
        .def("get_all_foods", &Environment::getAllFoods, "Get all food in the environment.")
        .def("simulate_iteration", &Environment::simulateIteration, py::arg("iterations"),
             py::arg("on_each_iteration") = nullptr)
        .def("get_dead_organisms", &Environment::getDeadOrganisms)
        .def("get_food_consumption_in_iteration", &Environment::getFoodConsumptionInIteration);

    py::register_exception<std::out_of_range>(m, "OutOfRangeException", PyExc_RuntimeError);
    py::register_exception<std::runtime_error>(m, "RuntimeException", PyExc_RuntimeError);
}
