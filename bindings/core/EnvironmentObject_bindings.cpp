#include <pybind11/pybind11.h>

#include <core/EnvironmentObject.hpp>

namespace py = pybind11;

// Trampoline class to allow Python subclassing of EnvironmentObject
class PyEnvironmentObject : public EnvironmentObject {
public:
    using EnvironmentObject::EnvironmentObject;

    void postIteration() override {
        PYBIND11_OVERRIDE(void, EnvironmentObject, postIteration);
    }
};

void init_EnvironmentObject(py::module &m) {
    py::class_<EnvironmentObject, PyEnvironmentObject, std::shared_ptr<EnvironmentObject>>(m, "EnvironmentObject")
        .def(py::init<float, float>())
        .def("get_position", &EnvironmentObject::getPosition)
        .def("get_id", &EnvironmentObject::getId)
        .def("set_position", &EnvironmentObject::setPosition)
        .def("post_iteration", &EnvironmentObject::postIteration);
}
