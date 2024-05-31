#ifndef BASE_ENVIRONMENT_OBJECT_HPP
#define BASE_ENVIRONMENT_OBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

class EnvironmentObject {
public:
    EnvironmentObject(float x, float y)
        : id(boost::uuids::random_generator()()), position(std::make_pair(x, y)) {}

    boost::uuids::uuid getId() const { return id; }

    virtual ~EnvironmentObject() = default;
    virtual void postIteration() {};

    // [TODO] change this - very bad implementation in order to make organism
    // move
    virtual std::pair<float, float> getPosition() const { return position; }
    virtual void setPosition(float x, float y) { position = std::make_pair(x, y); }

private:
    boost::uuids::uuid id;
    std::pair<float, float> position;
};

#endif
