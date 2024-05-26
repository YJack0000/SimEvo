#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>
#include <index/SpatialIndex.hpp>

template <typename T>
const int OptimizedSpatialIndex<T>::MAX_OBJECTS = 10;

template <typename T>
const int OptimizedSpatialIndex<T>::MIN_SIZE = 10;

template <typename T>
OptimizedSpatialIndex<T>::OptimizedSpatialIndex(int size)
    : size(size), isSubdivided(false) {}

template <typename T>
void OptimizedSpatialIndex<T>::insert(const T &object, float x, float y) {
    if (!inBounds(std::make_pair(x, y))) {
        return;
    }

    spatialObjects.push_back(SpatialObject<T>(object, x, y));
    if (spatialObjects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
    }

    this->addObjectPositionPair(object, x, y);
}

template <typename T>
std::vector<T> OptimizedSpatialIndex<T>::query(float x, float y, float range) {
    std::vector<T> result;
    if (!inBounds(std::make_pair(x, y))) {
        return result;
    }
    for (const auto &spaObj : spatialObjects) {
        auto pos = spaObj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        if (std::sqrt(dx * dx + dy * dy) <= range) {
            result.push_back(spaObj.getObject());
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

template <typename T>
void OptimizedSpatialIndex<T>::update(const T &object, float newX, float newY) {
    remove(object);
    insert(object, newX, newY);

    this->deleteObjectPositionPair(object);
    this->addObjectPositionPair(object, newX, newY);
}

template <typename T>
void OptimizedSpatialIndex<T>::remove(const T &object) {
    auto it = std::find_if(spatialObjects.begin(), spatialObjects.end(),
                           [&object](const SpatialObject<T> &o) {
                               return o.getObject() == object;
                           });

    if (it != spatialObjects.end()) {
        spatialObjects.erase(it);
    }
    if (isSubdivided) {
        for (const auto &child : children) {
            child->remove(object);
        }
        if (canMerge()) {
            merge();
        }
    }

    this->deleteObjectPositionPair(object);
}

template <typename T>
bool OptimizedSpatialIndex<T>::inBounds(const std::pair<float, float> &pos) {
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

    for (const auto &spaObj : spatialObjects) {
        for (auto &child : children) {
            child->insert(spaObj.getObject(), spaObj.getPosition().first,
                          spaObj.getPosition().second);
        }
    }

    spatialObjects.clear();
    isSubdivided = true;
}

template <typename T>
void OptimizedSpatialIndex<T>::setOffset(float offsetX, float offsetY) {
    offset = {offsetX, offsetY};
}

template <typename T>
bool OptimizedSpatialIndex<T>::canMerge() const {
    if (!isSubdivided) {
        return false;
    }
    for (const auto &child : children) {
        if (!child->isEmpty()) {
            return false;
        }
    }
    return true;
}

template <typename T>
void OptimizedSpatialIndex<T>::merge() {
    isSubdivided = false;
    for (auto &child : children) {
        child.reset();
    }
}

template <typename T>
bool OptimizedSpatialIndex<T>::isEmpty() const {
    return spatialObjects.empty() && !isSubdivided;
}

// make sure to instantiate the template class
template class OptimizedSpatialIndex<int>;
template class OptimizedSpatialIndex<float>;
template class OptimizedSpatialIndex<double>;
template class OptimizedSpatialIndex<std::string>;

template class OptimizedSpatialIndex<boost::uuids::uuid>;
