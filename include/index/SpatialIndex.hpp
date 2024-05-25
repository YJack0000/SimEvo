#ifndef SPATIALINDEX_HPP
#define SPATIALINDEX_HPP

#include <boost/uuid/uuid.hpp>
#include "ISpatialIndex.hpp"

class DefaultSpatialIndex : public ISpatialIndex {
public:
    DefaultSpatialIndex();
    void insert(const std::shared_ptr<ISpatialObject> &object) override;
    std::vector<std::shared_ptr<ISpatialObject>> query(float x, float y,
                                                       float range) override;

private:
    std::vector<std::shared_ptr<ISpatialObject>> objects;
};

class OptimizedSpatialIndex : public ISpatialIndex {
public:
    OptimizedSpatialIndex(int size);
    void insert(const std::shared_ptr<ISpatialObject> &object) override;
    std::vector<std::shared_ptr<ISpatialObject>> query(float x, float y,
                                                       float range) override;

private:
    int size;
    bool isSubdivided;
    std::vector<std::shared_ptr<ISpatialObject>> objects;
    std::unique_ptr<OptimizedSpatialIndex> children[4];
    std::pair<float, float> offset;

    static const int MAX_OBJECTS;
    static const int MIN_SIZE;

    bool inBounds(const std::pair<float, float> &pos);
    void setOffset(float offsetX, float offsetY);
    void subdivide();
};

#endif
