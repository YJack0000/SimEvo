#include <pybind11/pybind11.h>

#include <core/BaseEnvironmentObject.hpp>

namespace py = pybind11;

void init_BaseEnvironmentObject(py::module &m) {
    py::class_<BaseEnvironmentObject>(m, "BaseEnvironmentObject")
        .def(py::init<float, float>())
        .def("get_position", &BaseEnvironmentObject::getPosition)
        .def("get_id", &BaseEnvironmentObject::getId)
        .def("set_position", &BaseEnvironmentObject::setPosition)
        .def("post_iteration", &BaseEnvironmentObject::postIteration);
}
