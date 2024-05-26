#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>

#include "index/SpatialIndex.hpp"
#include "index/SpatialObject.hpp"

template <typename T>
DefaultSpatialIndex<T>::DefaultSpatialIndex() {}

template <typename T>
void DefaultSpatialIndex<T>::insert(const T &object, float x, float y) {
    spatialObjects.push_back(SpatialObject<T>(object, x, y));
    this->addObjectPositionPair(object, x, y);
}

template <typename T>
std::vector<T> DefaultSpatialIndex<T>::query(float x, float y, float range) {
    std::vector<T> result;
    for (const SpatialObject<T> &obj : this->spatialObjects) {
        auto pos = obj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        if (std::sqrt(dx * dx + dy * dy) <= range) {
            result.push_back(obj.getObject());  // Ensure you have a method to
                                                // get the internal object
        }
    }
    return result;
}

template <typename T>
void DefaultSpatialIndex<T>::update(const T &object, float newX, float newY) {
    auto it = std::find_if(spatialObjects.begin(), spatialObjects.end(),
                           [&object](const SpatialObject<T> &o) {
                               return o.getObject() ==
                                      object;  // Assuming you have a method to
                                               // get the internal object
                           });
    if (it != spatialObjects.end()) {
        it->setPosition(newX, newY);
    }

    this->deleteObjectPositionPair(object);
    this->addObjectPositionPair(object, newX, newY);
}

template <typename T>
void DefaultSpatialIndex<T>::remove(const T &object) {
    auto it = std::find_if(spatialObjects.begin(), spatialObjects.end(),
                           [&object](const SpatialObject<T> &o) {
                               return o.getObject() ==
                                      object;  // Assuming you have a method to
                                               // get the internal object
                           });
    if (it != spatialObjects.end()) {
        spatialObjects.erase(it);
    }

    this->deleteObjectPositionPair(object);
}

// make sure to instantiate the template class
template class DefaultSpatialIndex<int>;
template class DefaultSpatialIndex<float>;
template class DefaultSpatialIndex<double>;
template class DefaultSpatialIndex<std::string>;

template class DefaultSpatialIndex<boost::uuids::uuid>;
