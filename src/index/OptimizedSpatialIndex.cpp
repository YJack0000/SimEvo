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
    if (!inBounds(object.getPosition())) {
        return;
    }

    objects.push_back(object);
    if (objects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
    }

    addObjectPositionPair(object, x, y);
}

template <typename T>
std::vector<T> OptimizedSpatialIndex<T>::query(float x, float y, float range) {
    std::vector<T> result;
    if (!inBounds({static_cast<int>(x), static_cast<int>(y)})) {
        return result;
    }
    for (const auto &obj : objects) {
        auto pos = obj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        double distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= range) {
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

template <typename T>
void OptimizedSpatialIndex<T>::update(const T &object, float newX, float newY) {
    remove(object);
    T updatedObject = object;
    updatedObject.setPosition(newX, newY);
    insert(updatedObject);

    deleteObjectPositionPair(object);
    addObjectPositionPair(updatedObject, newX, newY);
}

template <typename T>
void OptimizedSpatialIndex<T>::remove(const T &object) {
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        objects.erase(it);
    }
    if (isSubdivided) {
        for (const auto &child : children) {
            child->remove(object);
        }
        if (canMerge()) {
            merge();
        }
    }

    deleteObjectPositionPair(object);
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
    for (const auto &obj : objects) {
        for (const auto &child : children) {
            child->insert(obj);
        }
    }
    objects.clear();
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
    return objects.empty() && !isSubdivided;
}
