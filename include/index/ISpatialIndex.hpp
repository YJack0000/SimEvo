#ifndef ISPATIALINDEX_HPP
#define ISPATIALINDEX_HPP

#include <memory>
#include <vector>

#include "ISpatialObject.hpp"

class ISpatialIndex {
public:
    virtual void insert(const std::shared_ptr<ISpatialObject> &objecit) = 0;
    virtual std::vector<std::shared_ptr<ISpatialObject>> query(int x, int y,
                                                               int range) = 0;
    virtual ~ISpatialIndex() = default;
};

#endif
