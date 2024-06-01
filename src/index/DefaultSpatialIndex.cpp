#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <cmath>
#include <index/DefaultSpatialIndex.hpp>
#include <stdexcept>

/**
 * @brief Constructs a new instance of DefaultSpatialIndex.
 */
template <typename T>
DefaultSpatialIndex<T>::DefaultSpatialIndex() {}

/**
 * @brief Inserts a new object into the spatial index.
 *
 * @param object The object to insert.
 * @param x The x-coordinate of the object.
 * @param y The y-coordinate of the object.
 * @throw std::invalid_argument Thrown if coordinates are out of bounds.
 */
template <typename T>
void DefaultSpatialIndex<T>::insert(const T &object, float x, float y) {
    spatialObjects.push_back(SpatialObject<T>(object, x, y));
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
std::vector<T> DefaultSpatialIndex<T>::query(float x, float y, float range) {
    std::vector<T> result;
    for (const SpatialObject<T> &obj : this->spatialObjects) {
        auto pos = obj.getPosition();
        float dx = pos.first - x;
        float dy = pos.second - y;
        // static unsigned int call = 0;
        // printf("Call %d\n", call++);
        if (std::sqrt(dx * dx + dy * dy) <= range) {
            result.push_back(obj.getObject());
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
 * @throw std::out_of_range Thrown if the object is not found.
 */
template <typename T>
void DefaultSpatialIndex<T>::update(const T &object, float newX, float newY) {
    auto it = findObject(object);
    if (it == spatialObjects.end()) {
        throw std::out_of_range("Object not found to update.");
    }

    it->setPosition(newX, newY);
}

/**
 * @brief Removes an object from the spatial index.
 *
 * @param object The object to remove.
 * @throw std::out_of_range Thrown if the object is not found.
 */
template <typename T>
void DefaultSpatialIndex<T>::remove(const T &object) {
    auto it = findObject(object);
    if (it == spatialObjects.end()) {
        throw std::out_of_range("Object not found to remove.");
    }

    spatialObjects.erase(it);
}

/**
 * @brief Clears the spatial index.
 */
template <typename T>
void DefaultSpatialIndex<T>::clear() {
    this->spatialObjects.clear();
}

/**
 * @brief Helper function to find an object in the spatialObjects vector.
 *
 * @param object The object to find.
 * @return An iterator to the found object, or end iterator if not found.
 */
template <typename T>
typename std::vector<SpatialObject<T>>::iterator DefaultSpatialIndex<T>::findObject(
    const T &object) {
    return std::find_if(spatialObjects.begin(), spatialObjects.end(),
                        [&object](const SpatialObject<T> &o) { return o.getObject() == object; });
}

// Instantiate the template class for required types
template class DefaultSpatialIndex<int>;
template class DefaultSpatialIndex<float>;
template class DefaultSpatialIndex<double>;
template class DefaultSpatialIndex<std::string>;
template class DefaultSpatialIndex<boost::uuids::uuid>;
