#ifndef SPATIALINDEX_HPP
#define SPATIALINDEX_HPP

#include "ISpatialIndex.hpp"

class DefaultSpatialIndex : public ISpatialIndex {
public:
    DefaultSpatialIndex();
    void insert(const std::shared_ptr<ISpatialObject> &object) override;
    std::vector<std::shared_ptr<ISpatialObject>> query(int x, int y,
                                                       int range) override;

private:
    std::vector<std::shared_ptr<ISpatialObject>> objects;
};

class OptimizedSpatialIndex : public ISpatialIndex {
public:
    OptimizedSpatialIndex(int size);
    void insert(const std::shared_ptr<ISpatialObject> &object) override;
    std::vector<std::shared_ptr<ISpatialObject>> query(int x, int y, int range) override;

private:
    int size;
    bool isSubdivided;
    std::vector<std::shared_ptr<ISpatialObject>> objects;
    std::unique_ptr<OptimizedSpatialIndex> children[4];
    std::pair<int, int> offset;

    static const int MAX_OBJECTS;
    static const int MIN_SIZE;

    bool inBounds(const std::pair<int, int> &pos);
    void subdivide();
    void setOffset(int offsetX, int offsetY);
};

#endif
