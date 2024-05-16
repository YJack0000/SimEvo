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
    OptimizedSpatialIndex();
    void insert(const std::shared_ptr<ISpatialObject> &object) override;
    std::vector<std::shared_ptr<ISpatialObject>> query(int x, int y,
                                                       int range) override;
};

#endif
