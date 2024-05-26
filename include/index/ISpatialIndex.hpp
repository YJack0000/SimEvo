#ifndef ISPATIALINDEX_HPP
#define ISPATIALINDEX_HPP

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <list>
#include <vector>

#include "SpatialObject.hpp"

namespace std {
template <>
struct hash<boost::uuids::uuid> {
    size_t operator()(const boost::uuids::uuid& uuid) const {
        return boost::hash<boost::uuids::uuid>()(uuid);
    }
};
}  // namespace std

template <typename T>
class ISpatialIndex {
public:
    virtual void insert(const T& object, float x, float y) = 0;
    virtual std::vector<T> query(float x, float y, float range) = 0;
    virtual void update(const T& object, float newX, float newY) = 0;
    virtual void remove(const T& object) = 0;
    virtual ~ISpatialIndex() = default;

    using iterator = typename std::list<T>::const_iterator;
    iterator begin() const { return objectPositions.begin(); }
    iterator end() const { return objectPositions.end(); }

    std::pair<float, float> getPosition(const T& object) {
        return objectPositions[object];
    }

protected:
    void addObjectPositionPair(const T& object, float x, float y) {
        objectPositions[object] = std::make_pair(x, y);
    }

    void deleteObjectPositionPair(const T& object) {
        objectPositions.erase(object);
    }

private:
    std::unordered_map<SpatialObject<T>, std::pair<float, float>>
        objectPositions;
};

#endif
