#include <boost/uuid/uuid.hpp>
#include <cmath>
#include <index/SpatialIndex.hpp>

template <typename T>
DefaultSpatialIndex<T>::DefaultSpatialIndex() {}

void DefaultSpatialIndex::insert(
    const std::shared_ptr<ISpatialObject>& object) {
    objects.push_back(object);
}

std::vector<std::shared_ptr<ISpatialObject>> DefaultSpatialIndex::query(float x, float y,
                                                           float range) {
    std::vector<std::shared_ptr<ISpatialObject>> result;
    for (const auto& obj : objects) {
        auto pos = obj->getPosition();
        if (std::abs(pos.first - x) <= range &&
            std::abs(pos.second - y) <= range) {
            result.push_back(obj);
        }
    }
    return result;
}
