#ifndef _ISPATIALOBJECT_HPP
#define _ISPATIALOBJECT_HPP

#include <string>

class ISpatialObject {
public:
    virtual std::pair<int, int> getPosition() const = 0;
    virtual std::string getType() const = 0;
    virtual ~ISpatialObject() {}
};

#endif
