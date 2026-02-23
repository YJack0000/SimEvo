#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>
#include <index/OptimizedSpatialIndex.hpp>
#include <sstream>

template <typename T>
const int OptimizedSpatialIndex<T>::MAX_OBJECTS = 10;

template <typename T>
const int OptimizedSpatialIndex<T>::MIN_SIZE = 10;

/**
 * @brief Constructs a new instance of OptimizedSpatialIndex with a specified size.
 * @param size The size of the index area.
 */
template <typename T>
OptimizedSpatialIndex<T>::OptimizedSpatialIndex(float size)
    : size(size), isSubdivided(false), offset(0, 0) {
    for (auto& child : children) {
        child = nullptr;
    }
}

/**
 * @brief Inserts a new object into the spatial index.
 *
 * @param object The object to insert.
 * @param x The x-coordinate of the object.
 * @param y The y-coordinate of the object.
 */
template <typename T>
void OptimizedSpatialIndex<T>::insert(const T& object, float x, float y) {
    if (!inBounds({x, y})) {
        std::stringstream ss;
        ss << "Insert coordinates (" << x << ", " << y << ") out of bounds. ";
        ss << "Size: " << size << " Offset: (" << offset.first << ", " << offset.second << ")";
        throw std::out_of_range(ss.str());
        return;
    }

    if (isSubdivided) {
        int childIndex = getChildIndex(x, y);
        if (childIndex != -1) {
            children[childIndex]->insert(object, x, y);
            return;
        }
    }

    spatialObjects.emplace_back(object, x, y);
    if (spatialObjects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
        for (const auto& obj : spatialObjects) {
            insert(obj.getObject(), obj.getPosition().first, obj.getPosition().second);
        }
        spatialObjects.clear();
    }
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
    _query(x, y, range, result);
    return result;
}

template <typename T>
void OptimizedSpatialIndex<T>::_query(float x, float y, float range, std::vector<T>& result) {
    if (!intersectsRange(x, y, range)) {
        return;
    }

    for (const auto& obj : spatialObjects) {
        if (getDistance(x, y, obj.getPosition().first, obj.getPosition().second) <= range) {
            result.push_back(obj.getObject());
        }
    }

    if (isSubdivided) {
        for (const auto& child : children) {
            if (child->intersectsRange(x, y, range)) {
                child->_query(x, y, range, result);
            }
        }
    }
}

/**
 * @brief Updates the position of an object in the spatial index.
 *
 * @param object The object to update.
 * @param newX The new x-coordinate of the object.
 * @param newY The new y-coordinate of the object.
 */
template <typename T>
void OptimizedSpatialIndex<T>::update(const T& object, float newX, float newY) {
    if (!inBounds(std::make_pair(newX, newY))) {
        std::stringstream ss;
        ss << "Update coordinates (" << newX << ", " << newY << ") out of bounds. ";
        ss << "Size: " << size;
        throw std::out_of_range(ss.str());
        return;
    }

    // Try to find the object in the current leaf and update in-place if it stays in the same leaf
    OptimizedSpatialIndex<T>* leaf = _findLeaf(newX, newY);
    if (leaf) {
        auto it = std::find_if(leaf->spatialObjects.begin(), leaf->spatialObjects.end(),
                               [&](const auto& obj) { return obj.getObject() == object; });
        if (it != leaf->spatialObjects.end()) {
            it->setPosition(newX, newY);
            return;
        }
    }

    // Object is not in the target leaf — must remove and reinsert
    remove(object);
    insert(object, newX, newY);
}

/**
 * @brief Removes an object from the spatial index.
 *
 * @param object The object to remove.
 */
template <typename T>
void OptimizedSpatialIndex<T>::remove(const T& object) {
    auto it = std::find_if(spatialObjects.begin(), spatialObjects.end(),
                           [&](const auto& obj) { return obj.getObject() == object; });

    if (it != spatialObjects.end()) {
        spatialObjects.erase(it);
        return;
    }

    if (isSubdivided) {
        for (auto& child : children) {
            child->remove(object);
            if (canMerge()) {
                merge();
                break;
            }
        }
    }
}

/**
 * @brief Clears the spatial index.
 */
template <typename T>
void OptimizedSpatialIndex<T>::clear() {
    spatialObjects.clear();
    if (isSubdivided) {
        for (auto& child : children) {
            child->clear();
        }
    }
    isSubdivided = false;
}

/**
 * @brief Helper function to check if a position is within the bounds of the spatial index.
 *
 * @param pos A pair of float representing the x and y coordinates.
 * @return true If the position is within bounds.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::inBounds(const std::pair<float, float>& pos) const {
    float x = pos.first;
    float y = pos.second;
    const float epsilon = 0.0001f;
    return x >= offset.first && x < offset.first + size + epsilon && y >= offset.second &&
           y < offset.second + size + epsilon;
}

/**
 * @brief Tests if a circle (cx, cy, range) intersects with this node's AABB.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::intersectsRange(float cx, float cy, float range) const {
    float closestX = std::max(offset.first, std::min(cx, offset.first + size));
    float closestY = std::max(offset.second, std::min(cy, offset.second + size));
    float dx = cx - closestX;
    float dy = cy - closestY;
    return (dx * dx + dy * dy) <= (range * range);
}

/**
 * @brief Subdivides the spatial index into smaller indices.
 */
template <typename T>
void OptimizedSpatialIndex<T>::subdivide() {
    float childSize = size / 2.0f;
    children[0] = std::make_unique<OptimizedSpatialIndex<T>>(childSize);
    children[1] = std::make_unique<OptimizedSpatialIndex<T>>(childSize);
    children[2] = std::make_unique<OptimizedSpatialIndex<T>>(childSize);
    children[3] = std::make_unique<OptimizedSpatialIndex<T>>(childSize);
    isSubdivided = true;
    setOffset(offset.first, offset.second);
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
    if (isSubdivided) {
        float childSize = size / 2;
        children[0]->setOffset(offsetX, offsetY);
        children[1]->setOffset(offsetX + childSize, offsetY);
        children[2]->setOffset(offsetX, offsetY + childSize);
        children[3]->setOffset(offsetX + childSize, offsetY + childSize);
    }
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

    int totalObjects = spatialObjects.size();
    for (const auto& child : children) {
        // if child has its own children, it can't be merged
        if (child->spatialObjects.size() == 0 && !child->isEmpty()) {
            return false;
        }

        totalObjects += child->spatialObjects.size();
    }

    return totalObjects < MAX_OBJECTS;
}

/**
 * @brief Merges all children back into this node, assuming they are all empty.
 */
template <typename T>
void OptimizedSpatialIndex<T>::merge() {
    isSubdivided = false;
    for (auto& child : children) {
        if (child) {
            spatialObjects.insert(spatialObjects.end(),
                                  std::make_move_iterator(child->spatialObjects.begin()),
                                  std::make_move_iterator(child->spatialObjects.end()));
            child.reset();
        }
    }
}

/**
 * @brief Checks if this spatial index is empty, meaning it has no objects and no active
 * subdivisions.
 *
 * @return true If there are no spatial objects and it is not subdivided.
 */
template <typename T>
bool OptimizedSpatialIndex<T>::isEmpty() const {
    return spatialObjects.empty() && !isSubdivided;
}

template <typename T>
int OptimizedSpatialIndex<T>::getChildIndex(float x, float y) const {
    if (!inBounds({x, y})) {
        return -1;
    }
    float childSize = size / 2;
    int childX = (x - offset.first) < childSize ? 0 : 1;
    int childY = (y - offset.second) < childSize ? 0 : 1;
    return childX + childY * 2;
}

template <typename T>
float OptimizedSpatialIndex<T>::getDistance(float x1, float y1, float x2, float y2) const {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Finds the leaf node that contains the given position.
 * Returns nullptr if the position is out of bounds.
 */
template <typename T>
OptimizedSpatialIndex<T>* OptimizedSpatialIndex<T>::_findLeaf(float x, float y) {
    if (!inBounds({x, y})) {
        return nullptr;
    }
    if (isSubdivided) {
        int childIndex = getChildIndex(x, y);
        if (childIndex != -1) {
            return children[childIndex]->_findLeaf(x, y);
        }
    }
    return this;
}

// make sure to instantiate the template class
template class OptimizedSpatialIndex<int>;
template class OptimizedSpatialIndex<float>;
template class OptimizedSpatialIndex<double>;
template class OptimizedSpatialIndex<std::string>;
template class OptimizedSpatialIndex<boost::uuids::uuid>;
