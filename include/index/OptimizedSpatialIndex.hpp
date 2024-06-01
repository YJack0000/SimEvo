#ifndef OPTIMIZED_SPATIAL_INDEX_HPP
#define OPTIMIZED_SPATIAL_INDEX_HPP

#include <memory>

#include "ISpatialIndex.hpp"

template <typename T>
class OptimizedSpatialIndex : public ISpatialIndex<T> {
public:
    OptimizedSpatialIndex(float size);
    void insert(const T& object, float x, float y) override;
    std::vector<T> query(float x, float y, float range) override;
    void update(const T& object, float newX, float newY) override;
    void remove(const T& object) override;
    void clear() override;
    ~OptimizedSpatialIndex() override = default;

    std::vector<std::shared_ptr<SpatialObject<T>>> spatialObjects;  // objects in this node
private:
    float size;
    bool isSubdivided;
    std::unique_ptr<OptimizedSpatialIndex<T>> children[4];
    std::pair<float, float> offset;

    static const int MAX_OBJECTS;
    static const int MIN_SIZE;

    void _query(float x, float y, float range, std::vector<T>& result);

    bool inBounds(const std::pair<float, float>& pos) const;
    void setOffset(float offsetX, float offsetY);
    void subdivide();
    bool canMerge() const;
    void merge();
    bool isEmpty() const;
    bool intersectsRange(float min_x, float max_x, float min_y, float max_y) const;
    int getChildIndex(float x, float y) const;
    float getDistance(float x1, float y1, float x2, float y2) const;
};

#endif
