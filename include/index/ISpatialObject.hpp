#ifndef ISPATIALOBJECT_HPP
#define ISPATIALOBJECT_HPP

#include <utility> 

class ISpatialObject {
public:
    virtual std::pair<int, int> getPosition() const = 0;
    virtual ~ISpatialObject() {}
};

#endif
