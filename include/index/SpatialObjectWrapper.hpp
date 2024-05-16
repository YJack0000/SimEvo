#ifndef SPATIAL_OBJECT_WRAPPER
#define SPATIAL_OBJECT_WRAPPER

#include <memory>
#include "ISpatialObject.hpp"

template <typename T>
class SpatialObjectWrapper : public ISpatialObject {
public:
    SpatialObjectWrapper(std::shared_ptr<T> obj, int x, int y)
        : object(obj), position(x, y) {}

    std::pair<int, int> getPosition() const override { return position; }

    std::shared_ptr<T> getObject() const { return object; }

private:
    std::shared_ptr<T> object;
    std::pair<int, int> position;
};

#endif
