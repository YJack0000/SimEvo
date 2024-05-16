#include <index/SpatialIndex.hpp>
#include <cmath>

DefaultSpatialIndex::DefaultSpatialIndex() {}

void DefaultSpatialIndex::insert(const std::shared_ptr<ISpatialObject> &object) {
    objects.push_back(object);
}

std::vector<std::shared_ptr<ISpatialObject>> DefaultSpatialIndex::query(int x, int y, int range) {
    std::vector<std::shared_ptr<ISpatialObject>> result;

    for (const auto &obj : this->objects) {
        auto pos = obj->getPosition();
        int dx = pos.first - x;
        int dy = pos.second - y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= range) {
            result.push_back(obj);
        }
    }

    return result;
}
