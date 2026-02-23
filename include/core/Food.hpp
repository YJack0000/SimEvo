#ifndef FOOD_HPP
#define FOOD_HPP

#include <atomic>

#include "EnvironmentObject.hpp"

/**
 * @brief Represents the lifecycle state of a Food object.
 */
enum class FoodState { FRESH, EATEN };

/**
 * @brief A consumable food item that organisms can eat to gain energy.
 *
 * Food objects exist in the environment and provide a fixed amount of energy
 * when consumed. Once eaten, the food transitions to the EATEN state and will
 * be cleaned up by the environment. Uses atomic state to allow safe concurrent
 * reads during the parallelizable reaction phase.
 */
class Food : public EnvironmentObject {
public:
    /** @brief Construct a Food object at the origin with default energy (500). */
    Food() : EnvironmentObject(0, 0) {}

    /**
     * @brief Check whether this food is still available for consumption.
     * @return true if the food has not yet been eaten.
     */
    bool canBeEaten() const { return state.load(std::memory_order_acquire) == FoodState::FRESH; }

    /**
     * @brief Mark this food as eaten, transitioning it to the EATEN state.
     *
     * Uses release memory ordering so that the state change is visible to
     * concurrent readers in the reaction phase.
     */
    void eaten() { state.store(FoodState::EATEN, std::memory_order_release); }

    /**
     * @brief Get the energy value this food provides when consumed.
     * @return Fixed energy value (500) added to the consuming organism's lifespan.
     */
    int getEnergy() const { return 500; }

private:
    /// Atomic state ensures safe concurrent access from the parallel reaction phase.
    std::atomic<FoodState> state{FoodState::FRESH};
};

#endif
