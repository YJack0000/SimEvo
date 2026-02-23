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
 * each iteration. The interaction phase runs single-threaded because it mutates
 * shared state (food eaten flags, organism lifespans). The reaction phase can
 * be parallelized since each organism only writes to its own movement fields.
 */
class Environment {
public:
    /**
     * @brief Construct an environment with the given dimensions and spatial index type.
     * @param width  The horizontal extent of the simulation area.
     * @param height The vertical extent of the simulation area.
     * @param type   Spatial index implementation: "default" or "optimized".
     * @param numThreads Number of threads for the parallelizable reaction phase.
     * @throws std::invalid_argument If type is not "default" or "optimized".
     */
    Environment(int width, int height, std::string type = "default", int numThreads = 1);

    /** @brief Get the horizontal extent of the environment. */
    int getWidth() const { return width; }

    /** @brief Get the vertical extent of the environment. */
    int getHeight() const { return height; }

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

    /** @brief Clear all objects, dead organisms, and counters from the environment. */
    void reset();

    /**
     * @brief Run the simulation for a given number of iterations.
     * @param iterations Number of iterations to simulate.
     * @param on_each_iteration Optional callback invoked after each iteration.
     *
     * Each iteration proceeds in order: interactions (single-threaded),
     * reactions (optionally multi-threaded), then post-iteration (life
     * consumption + movement). Stops early if no organisms or food remain.
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

    int numThreads = 1;  ///< Thread count for the parallelizable reaction phase

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
     * Safe to parallelize because each organism only writes to its own
     * movement vector and reactionCounter. Uses numThreads worker threads
     * when numThreads > 1.
     */
    void handleReactions();

    /** @brief Run post-iteration: deduct life consumption, move organisms, update spatial index. */
    void postIteration();

    /** @brief Remove dead organisms and eaten food from the active object map. */
    void cleanUp();
};

#endif
