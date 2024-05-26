#ifndef SPATIALINDEX_HPP
#define SPATIALINDEX_HPP

#include "ISpatialIndex.hpp"
#include "SpatialObject.hpp"

template <typename T>
class DefaultSpatialIndex : public ISpatialIndex<T> {
public:
    DefaultSpatialIndex();
    void insert(const T& object, float x, float y) override;
    std::vector<T> query(float x, float y, float range) override;
    void update(const T& object, float newX, float newY) override;
    void remove(const T& object) override;

private:
    std::vector<SpatialObject<T>> spatialObjects;
};

template <typename T>
class OptimizedSpatialIndex : public ISpatialIndex<T> {
public:
    OptimizedSpatialIndex(int size);
    void insert(const T& object, float x, float y) override;
    std::vector<T> query(float x, float y, float range) override;
    void update(const T& object, float newX, float newY) override;
    void remove(const T& object) override;
    ~OptimizedSpatialIndex() override = default;

private:
    int size;
    bool isSubdivided;
    std::vector<SpatialObject<T>> spatialObjects;  // objects in this node
    std::unique_ptr<OptimizedSpatialIndex<T>> children[4];
    std::pair<float, float> offset;

    static const int MAX_OBJECTS;
    static const int MIN_SIZE;

    bool inBounds(const std::pair<float, float>& pos);
    void setOffset(float offsetX, float offsetY);
    void subdivide();
    bool canMerge() const;
    void merge();
    bool isEmpty() const;
};

#endif
