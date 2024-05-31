#ifndef ISPATIALINDEX_HPP
#define ISPATIALINDEX_HPP

#include <list>
#include <vector>

template <typename T>
class SpatialObject {
public:
    SpatialObject(const T& object, float x, float y)
        : object(object), position(std::make_pair(x, y)) {}

    T getObject() const { return object; }
    std::pair<float, float> getPosition() const { return position; }
    void setPosition(float x, float y) { position = std::make_pair(x, y); }

private:
    T object;
    std::pair<float, float> position;
};

template <typename T>
class ISpatialIndex {
public:
    virtual void insert(const T& object, float x, float y) = 0;
    virtual std::vector<T> query(float x, float y, float range) = 0;
    virtual void update(const T& object, float newX, float newY) = 0;
    virtual void remove(const T& object) = 0;
    virtual void clear() = 0;
    virtual ~ISpatialIndex() = default;

    using iterator = typename std::list<T>::const_iterator;
    iterator begin() const { return objectPositions.begin(); }
    iterator end() const { return objectPositions.end(); }

    std::pair<float, float> getPosition(const T& object) { return objectPositions[object]; }

protected:
    void addObjectPositionPair(const T& object, float x, float y) {
        objectPositions[object] = std::make_pair(x, y);
    }

    void deleteObjectPositionPair(const T& object) { objectPositions.erase(object); }
    void clearObjectPositionPairs() { objectPositions.clear(); }

private:
    std::unordered_map<T, std::pair<float, float>> objectPositions;
};

#endif
