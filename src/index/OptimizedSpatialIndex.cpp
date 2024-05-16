#include <index/SpatialIndex.hpp>
#include <cmath>

const int OptimizedSpatialIndex::MAX_OBJECTS = 10;
const int OptimizedSpatialIndex::MIN_SIZE = 10;

OptimizedSpatialIndex::OptimizedSpatialIndex(int size)
    : size(size), isSubdivided(false) {}

void OptimizedSpatialIndex::insert(const std::shared_ptr<ISpatialObject> &object) {
    if (!inBounds(object->getPosition())) {
        return;
    }

    objects.push_back(object);

    if (objects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
    }
}

std::vector<std::shared_ptr<ISpatialObject>> OptimizedSpatialIndex::query(int x, int y, int range) {
    std::vector<std::shared_ptr<ISpatialObject>> result;

    if (!inBounds({x, y})) {
        return result;
    }

    for (const auto &obj : objects) {
        auto pos = obj->getPosition();
        int dx = pos.first - x;
        int dy = pos.second - y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (static_cast<int>(distance) <= range) {
            result.push_back(obj);
        }
    }

    if (isSubdivided) {
        for (const auto &child : children) {
            auto childResult = child->query(x, y, range);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
    }

    return result;
}

bool OptimizedSpatialIndex::inBounds(const std::pair<int, int> &pos) {
    return pos.first >= 0 && pos.first < size && pos.second >= 0 && pos.second < size;
}

void OptimizedSpatialIndex::subdivide() {
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
        children[i] = std::make_unique<OptimizedSpatialIndex>(childSize);
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

void OptimizedSpatialIndex::setOffset(int offsetX, int offsetY) {
    offset = {offsetX, offsetY};
}
