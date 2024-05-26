#include <cmath>
#include <index/SpatialIndex.hpp>

template <typename T>
DefaultSpatialIndex<T>::DefaultSpatialIndex() {}

template <typename T>
void DefaultSpatialIndex<T>::insert(const T& object, float x, float y) {
    objects.push_back(SpatialObject<T>(object, x, y));
    this->addObjectPositionPair(object, x, y);
}

template <typename T>
std::vector<T> DefaultSpatialIndex<T>::query(float x, float y, float range) {
    std::vector<T> result;

    for (const auto& obj : this->objects) {
        auto pos = obj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        double distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= range) {
            result.push_back(obj);
        }
    }

    return result;
}

template <typename T>
void DefaultSpatialIndex<T>::update(const T& object, float newX, float newY) {
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        T& obj = *it;
        obj.setPosition(newX, newY);
    }

    // this->deleteObjectPositionPair(object);
    // this->addObjectPositionPair(object, newX, newY);
}

template <typename T>
void DefaultSpatialIndex<T>::remove(const T& object) {
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        objects.erase(it);
    }

    // this->deletObjectPositionPair(object);
}

