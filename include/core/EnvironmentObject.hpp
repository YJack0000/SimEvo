#ifndef BASE_ENVIRONMENT_OBJECT_HPP
#define BASE_ENVIRONMENT_OBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "Vec2.hpp"

class EnvironmentObject {
public:
    EnvironmentObject(float x, float y)
        : id(boost::uuids::random_generator()()), position(x, y) {}

    boost::uuids::uuid getId() const { return id; }

    virtual ~EnvironmentObject() = default;
    virtual void postIteration() {}

    std::pair<float, float> getPosition() const { return position; }
    void setPosition(float x, float y) { position = Vec2(x, y); }

    Vec2 getPos() const { return position; }
    void setPos(Vec2 pos) { position = pos; }

private:
    boost::uuids::uuid id;

protected:
    Vec2 position;
};

#endif
