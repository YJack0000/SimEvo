#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>
#include <cstdio>
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

    spatialObjects.push_back(std::make_shared<SpatialObject<T>>(object, x, y));
    if (spatialObjects.size() > MAX_OBJECTS && size > MIN_SIZE) {
        subdivide();
        for (const auto& obj : spatialObjects) {
            insert(obj->getObject(), obj->getPosition().first, obj->getPosition().second);
        }
        spatialObjects.clear();
    }
}

/**
 * @brief Queries the spatial index for objects within a specified range.
 *
 * Cause it is a range query, we need to check all children if they intersect the query range.
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
    if (!inBounds({x, y})) {
        return;
    }

    // printf("====================================\n");
    // printf("Current node size: %ld\n", spatialObjects.size());
    for (const auto& obj : spatialObjects) {
        // printf("====================================\n");
        // printf("x: %f %f\n", x, y);
        // printf("obj: %f %f\n", obj.getPosition().first, obj.getPosition().second);
        // printf("range: %f, distance: %f\n", range,
        //        getDistance(x, y, obj.getPosition().first, obj.getPosition().second));
        if (getDistance(x, y, obj->getPosition().first, obj->getPosition().second) <= range) {
            result.push_back(obj->getObject());
        }
    }

    // printf("====================================\n");
    // printf("Have children: %d\n", isSubdivided);
    if (isSubdivided) {  // need to check children
        // printf("------------------------------------\n");
        // printf("x: %f %f\n", x, y);
        for (const auto& child : children) {
            child->_query(x, y, range, result);
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
                           [&](const auto& obj) { return obj->getObject() == object; });

    if (it != spatialObjects.end()) {
        // auto rand = std::rand() % 100;
        // printf("Before erase: %d %ld\n", rand, spatialObjects.size());
        spatialObjects.erase(it);
        // printf("After erase: %d %ld\n", rand, spatialObjects.size());
        return;
    }

    if (isSubdivided) {
        for (auto& child : children) {
            // auto rand = std::rand() % 100;
            // printf("Before remove: %d %ld\n", rand, child->spatialObjects.size());
            child->remove(object);
            // printf("After remove: %d %ld\n", rand, child->spatialObjects.size());
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
            printf("Clearing child\n");
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

    // printf("------------------------------------\n");
    // printf("Checking merge\n");
    // printf("Current node size: %ld\n", spatialObjects.size());

    int totalObjects = spatialObjects.size();
    for (const auto& child : children) {
        // printf("Child node size: %ld\n", child->spatialObjects.size());
        // printf("Does child have objects: %d\n", !child->isEmpty());

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
            /*printf("Before clear: %ld\n", child->spatialObjects.size());*/
            // child->clear();
            child.reset();
        }
    }
    // printf("After clear: %ld\n", spatialObjects.size());
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
    // static unsigned long called = 0;
    // printf("Called: %ld\n", called++);
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

// make sure to instantiate the template class
template class OptimizedSpatialIndex<int>;
template class OptimizedSpatialIndex<float>;
template class OptimizedSpatialIndex<double>;
template class OptimizedSpatialIndex<std::string>;
template class OptimizedSpatialIndex<boost::uuids::uuid>;
