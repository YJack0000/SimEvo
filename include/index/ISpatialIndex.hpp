#ifndef ISPATIALINDEX_HPP
#define ISPATIALINDEX_HPP

#include <list>
#include <memory>
#include <vector>

template <typename T>
class ISpatialIndex {
public:
    virtual void insert(const std::shared_ptr<ISpatialObject>& objecit) = 0;
    virtual std::vector<std::shared_ptr<ISpatialObject>> query(float x, float y, float range) = 0;
    virtual ~ISpatialIndex() = default;

    using iterator = std::list<std::shared_ptr<ISpatialObject>>::const_iterator;
    iterator begin() const { return objects.begin(); }
    iterator end() const { return objects.end(); }

protected:
    void addObject(const std::shared_ptr<ISpatialObject>& object) {
        objects.push_back(object);
    }

private:
    std::list<std::shared_ptr<ISpatialObject>> objects;
};

#endif
