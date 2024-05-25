#ifndef ISPATIALOBJECT_HPP
#define ISPATIALOBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <utility>

class ISpatialObject {
public:
    virtual boost::uuids::uuid getId() const = 0;
    virtual std::pair<float, float> getPosition() const = 0;
    virtual std::shared_ptr<void> getObject() const = 0;
    virtual ~ISpatialObject() {}
};

#endif
