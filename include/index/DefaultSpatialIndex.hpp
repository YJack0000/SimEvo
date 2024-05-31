#ifndef DEFAULT_SPATIAL_INDEX_HPP
#define DEFAULT_SPATIAL_INDEX_HPP

#include "ISpatialIndex.hpp"

template <typename T>
class DefaultSpatialIndex : public ISpatialIndex<T> {
public:
    DefaultSpatialIndex();
    void insert(const T& object, float x, float y) override;
    std::vector<T> query(float x, float y, float range) override;
    void update(const T& object, float newX, float newY) override;
    void remove(const T& object) override;
    void clear() override;

private:
    std::vector<SpatialObject<T>> spatialObjects;
    typename std::vector<SpatialObject<T>>::iterator findObject(const T& object);
};

#endif
