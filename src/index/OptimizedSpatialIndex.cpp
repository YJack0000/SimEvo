#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>
#include <index/OptimizedSpatialIndex.hpp>

template <typename T>
const int OptimizedSpatialIndex<T>::MAX_OBJECTS = 10;

template <typename T>
const int OptimizedSpatialIndex<T>::MIN_SIZE = 10;

/**
 * @brief Constructs a new instance of OptimizedSpatialIndex with a specified size.
 * @param size The size of the index area.
 */
template <typename T>
OptimizedSpatialIndex<T>::OptimizedSpatialIndex(int size) : size(size), isSubdivided(false) {}

/**
 * @brief Inserts a new object into the spatial index.
 *
 * @param object The object to insert.
 * @param x The x-coordinate of the object.
 * @param y The y-coordinate of the object.
 */
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

/**
 * @brief Queries the spatial index for objects within a specified range.
 *
 * @param x The x-coordinate of the query center.
 * @param y The y-coordinate of the query center.
 * @param range The query radius.
 * @return std::vector<T> A list of objects within the range.
 */
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

/**
 * @brief Updates the position of an object in the spatial index.
 *
 * @param object The object to update.
 * @param newX The new x-coordinate of the object.
 * @param newY The new y-coordinate of the object.
 */
template <typename T>
void OptimizedSpatialIndex<T>::update(const T &object, float newX, float newY) {
    remove(object);
    insert(object, newX, newY);

    this->deleteObjectPositionPair(object);
    this->addObjectPositionPair(object, newX, newY);
}

/**
 * @brief Removes an object from the spatial index.
 *
 * @param object The object to remove.
 */
template <typename T>
void OptimizedSpatialIndex<T>::remove(const T &object) {
    auto it =
        std::find_if(spatialObjects.begin(), spatialObjects.end(),
                     [&object](const SpatialObject<T> &o) { return o.getObject() == object; });

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

/**
 * @brief Clears the spatial index.
 */
template <typename T>
void OptimizedSpatialIndex<T>::clear() {
    spatialObjects.clear();
    
    // Clear each child and then reset the unique_ptr
    for (auto &child : children) {
        if (child) {
            child->clear();  // Ensure child clears its own resources
            child.reset();   // Release the object and set pointer to nullptr
        }
    }

    this->clearObjectPositionPairs();
}

/**
 * @brief Helper function to check if a position is within the bounds of the spatial index.
 *
 * @param pos A pair of float representing the x and y coordinates.
 * @return true If the position is within bounds.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::inBounds(const std::pair<float, float> &pos) {
    return pos.first >= 0 && pos.first < size && pos.second >= 0 && pos.second < size;
}

/**
 * @brief Subdivides the spatial index into smaller indices.
 */
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

/**
 * @brief Sets the offset for this spatial index, used when subdivided.
 *
 * @param offsetX The offset in the x-coordinate.
 * @param offsetY The offset in the y-coordinate.
 */
template <typename T>
void OptimizedSpatialIndex<T>::setOffset(float offsetX, float offsetY) {
    offset = std::make_pair(offsetX, offsetY);
}

/**
 * @brief Checks if it is possible to merge children back into this node.
 *
 * @return true If all children are empty and the subdivision can be reversed.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::canMerge() const {
    if (!isSubdivided) {
        return false;
    }
    for (const auto &child : children) {
        if (child && !child->isEmpty()) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Merges all children back into this node, assuming they are all empty.
 */
template <typename T>
void OptimizedSpatialIndex<T>::merge() {
    if (canMerge()) {
        for (auto &child : children) {
            child.reset();
        }
        isSubdivided = false;
    }
}

/**
 * @brief Checks if this spatial index is empty, meaning it has no objects and no active subdivisions.
 *
 * @return true If there are no spatial objects and it is not subdivided.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::isEmpty() const {
    if (!spatialObjects.empty()) {
        return false;
    }
    if (isSubdivided) {
        for (const auto &child : children) {
            if (child && !child->isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

// make sure to instantiate the template class
template class OptimizedSpatialIndex<int>;
template class OptimizedSpatialIndex<float>;
template class OptimizedSpatialIndex<double>;
template class OptimizedSpatialIndex<std::string>;
template class OptimizedSpatialIndex<boost::uuids::uuid>;
