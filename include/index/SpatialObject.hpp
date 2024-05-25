#ifndef SPATIAL_OBJECT_HPP
#define SPATIAL_OBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <utility>

template <typename T>
class SpatialObject {
public:
    SpatialObject(const T& object, float x, float y) :
        object(object),
        position(std::make_pair(x, y)) {}

    std::pair<int, int> getPosition() const { return position; }
    void setPosition(int x, int y) { position = std::make_pair(x, y); }

private:
    T object;
    std::pair<int, int> position;
};

#endif
