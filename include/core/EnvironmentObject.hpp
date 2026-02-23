#ifndef BASE_ENVIRONMENT_OBJECT_HPP
#define BASE_ENVIRONMENT_OBJECT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "Vec2.hpp"

/**
 * @brief Base class for all objects that exist in the simulation environment.
 *
 * Each object has a unique UUID identifier and a 2D position. Derived classes
 * include Organism and Food. Position is stored as Vec2 and is accessible
 * both as Vec2 (getPos/setPos) and as std::pair (getPosition/setPosition)
 * for backward compatibility.
 */
class EnvironmentObject {
public:
    /**
     * @brief Construct an environment object at the given coordinates.
     * @param x Initial x position.
     * @param y Initial y position.
     */
    EnvironmentObject(float x, float y)
        : id(boost::uuids::random_generator()()), position(x, y) {}

    /** @brief Get the unique identifier of this object. */
    boost::uuids::uuid getId() const { return id; }

    virtual ~EnvironmentObject() = default;

    /** @brief Called at the end of each simulation iteration. Override in subclasses. */
    virtual void postIteration() {}

    /** @brief Get position as a std::pair (legacy interface). */
    std::pair<float, float> getPosition() const { return position; }

    /**
     * @brief Set position from individual coordinates.
     * @param x New x coordinate.
     * @param y New y coordinate.
     */
    void setPosition(float x, float y) { position = Vec2(x, y); }

    /** @brief Get position as a Vec2. */
    Vec2 getPos() const { return position; }

    /** @brief Set position from a Vec2. */
    void setPos(Vec2 pos) { position = pos; }

private:
    boost::uuids::uuid id;  ///< Unique identifier for spatial-index lookups

protected:
    Vec2 position;  ///< Current position (accessible to subclasses)
};

#endif
