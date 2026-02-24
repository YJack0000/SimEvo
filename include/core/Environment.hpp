#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <memory>
#include <unordered_map>

#include "Food.hpp"
#include "Organism.hpp"
#include "index/ISpatialIndex.hpp"

/**
 * @brief The simulation world that manages organisms, food, and spatial queries.
 *
 * Environment owns all simulation objects, delegates spatial lookups to an
 * ISpatialIndex implementation, and drives the interact-react-move lifecycle
 * each iteration. Both phases currently run single-threaded so that Python
 * strategy callbacks can safely acquire the GIL.
 *
 * TODO: Re-enable multi-threaded handleReactions(). The reaction phase only
 * writes to each organism's own movement fields and is inherently parallelizable.
 * When Python callbacks are involved, either release the GIL before spawning
 * worker threads (using py::gil_scoped_release) or detect at runtime whether
 * any organism has a custom strategy and only fall back to single-threaded in
 * that case.
 */
class Environment {
public:
    /**
     * @brief Construct an environment with the given dimensions and spatial index type.
     * @param width  The horizontal extent of the simulation area.
     * @param height The vertical extent of the simulation area.
     * @param type   Spatial index implementation: "default" or "optimized".
     * @param numThreads Reserved for future multi-threaded reaction phase.
     * @throws std::invalid_argument If type is not "default" or "optimized".
     */
    Environment(int width, int height, std::string type = "default", int numThreads = 1);

    /** @brief Get the horizontal extent of the environment. */
    int getWidth() const { return width; }

    /** @brief Get the vertical extent of the environment. */
    int getHeight() const { return height; }

  
    void setVerbose(bool verbose) { this->verbose = verbose; }
  
    /**
     * @brief Add an organism to the environment at the specified coordinates.
     * @param organism Shared pointer to the organism.
     * @param x Horizontal position.
     * @param y Vertical position.
     * @throws std::out_of_range If (x, y) is outside the environment bounds.
     */
    void add(const std::shared_ptr<Organism>& organism, float x, float y);

    /**
     * @brief Add a food item to the environment at the specified coordinates.
     * @param food Shared pointer to the food.
     * @param x Horizontal position.
     * @param y Vertical position.
     * @throws std::out_of_range If (x, y) is outside the environment bounds.
     */
    void add(const std::shared_ptr<Food>& food, float x, float y);

    /**
     * @brief Add a generic EnvironmentObject (e.g. a Python subclass) at the specified coordinates.
     * @param object Shared pointer to the environment object.
     * @param x Horizontal position.
     * @param y Vertical position.
     * @throws std::out_of_range If (x, y) is outside the environment bounds.
     */
    void add(const std::shared_ptr<EnvironmentObject>& object, float x, float y);

    /**
     * @brief Remove an organism from the environment and spatial index.
     * @param organism Shared pointer to the organism to remove.
     * @throws std::runtime_error If the organism is not found.
     */
    void remove(const std::shared_ptr<Organism>& organism);

    /**
     * @brief Remove a food item from the environment and spatial index.
     * @param food Shared pointer to the food to remove.
     * @throws std::runtime_error If the food is not found.
     */
    void remove(const std::shared_ptr<Food>& food);

    /**
     * @brief Remove a generic EnvironmentObject from the environment and spatial index.
     * @param object Shared pointer to the object to remove.
     * @throws std::runtime_error If the object is not found.
     */
    void remove(const std::shared_ptr<EnvironmentObject>& object);

    /** @brief Clear all objects, dead organisms, and counters from the environment. */
    void reset();

    /**
     * @brief Run the simulation for a given number of iterations.
     * @param iterations Number of iterations to simulate.
     * @param on_each_iteration Optional callback invoked after each iteration.
     *
     * Each iteration proceeds in order: interactions, reactions, then
     * post-iteration (life consumption + movement). Stops early if no
     * organisms or food remain.
     */
    void simulateIteration(int iterations,
                           std::function<void(const Environment&)> on_each_iteration = nullptr);

    /** @brief Get all living organisms currently in the environment. */
    std::vector<std::shared_ptr<Organism>> getAllOrganisms() const;

    /** @brief Get all food items currently in the environment. */
    std::vector<std::shared_ptr<Food>> getAllFoods() const;

    /** @brief Get all environment objects (organisms and food). */
    std::vector<std::shared_ptr<EnvironmentObject>> getAllObjects() const;

    /** @brief Get organisms that died during the simulation run. */
    std::vector<std::shared_ptr<Organism>> getDeadOrganisms() const;

    /** @brief Get the total number of food items consumed across all iterations. */
    unsigned long getFoodConsumptionInIteration() const;

private:
    int width, height;
    std::string type;  ///< Spatial index type identifier ("default" or "optimized")
    std::unique_ptr<ISpatialIndex<boost::uuids::uuid>> spatialIndex;
    /// Maps object UUIDs to their shared pointers for O(1) lookup
    std::unordered_map<boost::uuids::uuid, std::shared_ptr<EnvironmentObject>> objectsMapper;

    std::vector<std::shared_ptr<Organism>> deadOrganisms;  ///< Accumulated dead organisms
    unsigned long foodConsumption = 0;                      ///< Running food consumption counter

    // TODO: use numThreads to re-enable multi-threaded handleReactions()
    int numThreads = 1;  ///< Reserved for future multi-threaded reaction phase
    bool verbose = false;

    /**
     * @brief Validate that coordinates fall within environment bounds.
     * @param x Horizontal coordinate.
     * @param y Vertical coordinate.
     * @throws std::out_of_range If coordinates are outside [0, width] x [0, height].
     */
    void checkBounds(float x, float y) const;

    /** @brief Sync organism positions into the spatial index, clamping to bounds. */
    void updatePositionsInSpatialIndex();

    /**
     * @brief Run the interaction phase: organisms eat food and fight.
     *
     * Runs single-threaded because interactions mutate shared state (food
     * eaten flags, other organisms' lifespans) which would cause data races.
     */
    void handleInteractions();

    /**
     * @brief Run the reaction phase: organisms decide movement direction.
     *
     * Currently runs single-threaded for GIL safety with Python callbacks.
     * TODO: Re-enable multi-threading -- each organism only writes to its own
     * movement/reactionCounter fields, so this phase is inherently parallelizable.
     */
    void handleReactions();

    /** @brief Run post-iteration: deduct life consumption, move organisms, update spatial index. */
    void postIteration();

    /** @brief Remove dead organisms and eaten food from the active object map. */
    void cleanUp();
};

#endif
