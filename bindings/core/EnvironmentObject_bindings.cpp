#include <pybind11/pybind11.h>

#include <core/EnvironmentObject.hpp>

namespace py = pybind11;

void init_EnvironmentObject(py::module &m) {
    py::class_<EnvironmentObject, std::shared_ptr<EnvironmentObject>>(m, "EnvironmentObject")
        .def(py::init<float, float>())
        .def("get_position", &EnvironmentObject::getPosition)
        .def("get_id", &EnvironmentObject::getId)
        .def("set_position", &EnvironmentObject::setPosition)
        .def("post_iteration", &EnvironmentObject::postIteration);
}
