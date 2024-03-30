#include "SpatialObjectContainer.hpp"

SpatialObjectContainer::SpatialObjectContainer(std::shared_ptr<void> obj, int x,
                                               int y, std::string objType)
    : object(obj), position(x, y), type(objType) {}

std::pair<int, int> SpatialObjectContainer::getPosition() const {
  return position;
}

std::string SpatialObjectContainer::getType() const { return type; }

template <typename T>
std::shared_ptr<T> SpatialObjectContainer::getObject() const {
  return std::static_pointer_cast<T>(object);
}
