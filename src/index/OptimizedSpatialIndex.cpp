#include <cmath>
#include <index/SpatialIndex.hpp>

template <typename T>
const int OptimizedSpatialIndex<T>::MAX_OBJECTS = 10;

template <typename T>
const int OptimizedSpatialIndex<T>::MIN_SIZE = 10;

template <typename T>
OptimizedSpatialIndex<T>::OptimizedSpatialIndex(int size)
    : size(size), isSubdivided(false) {}

void OptimizedSpatialIndex::insert(
    const std::shared_ptr<ISpatialObject> &object) {
    if (!inBounds(object->getPosition())) {
        return;
    }
    objects.push_back(object);
    if (objects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
    }
}

std::vector<std::shared_ptr<ISpatialObject>> OptimizedSpatialIndex::query(
    float x, float y, float range) {
    std::vector<std::shared_ptr<ISpatialObject>> result;

    if (!inBounds({x, y})) {
        return result;
    }
    for (const auto &obj : objects) {
        auto pos = obj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (static_cast<float>(distance) <= range) {
            result.push_back(obj);
        }
    }
    if (isSubdivided) {
        for (const auto &child : children) {
            std::vector<std::shared_ptr<ISpatialObject>> childResult =
                child->query(x, y, range);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
    }
    return result;
}

bool OptimizedSpatialIndex::inBounds(const std::pair<float, float> &pos) {
    return pos.first >= 0 && pos.first < size && pos.second >= 0 &&
           pos.second < size;
}

template <typename T>
void OptimizedSpatialIndex<T>::subdivide() {
    int childSize = size / 2;
    int childX = 0;
    int childY = 0;
    for (int i = 0; i < 4; ++i) {
        if (i == 1 || i == 3) {
            childX = childSize;
        }
        if (i == 2 || i == 3) {
            childY = childSize;
        }
        children[i] = std::make_unique<OptimizedSpatialIndex<T>>(childSize);
        children[i]->setOffset(childX, childY);
    }
    for (const auto &obj : objects) {
        for (const auto &child : children) {
            child->insert(obj);
        }
    }
    objects.clear();
    isSubdivided = true;
}

void OptimizedSpatialIndex::setOffset(float offsetX, float offsetY) {
    offset = {offsetX, offsetY};
}
