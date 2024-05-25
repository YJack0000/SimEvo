#ifndef SPATIAL_OBJECT_WRAPPER
#define SPATIAL_OBJECT_WRAPPER

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>

#include "ISpatialObject.hpp"

template <typename T>
class SpatialObjectWrapper : public ISpatialObject {
public:
    SpatialObjectWrapper(std::shared_ptr<T> obj, float x, float y)
        : object(obj), position(x, y), id(boost::uuids::random_generator()()) {}

    boost::uuids::uuid getId() const override { return id; }
    std::pair<float, float> getPosition() const override { return position; }
    std::shared_ptr<T> getObject() const { return object; }

private:
    boost::uuids::uuid id;

    std::shared_ptr<T> object;
    std::pair<float, float> position;
};

#endif
