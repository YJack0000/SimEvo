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
 * Food objects exist in the environment and provide energy when consumed. Once
 * eaten, the food transitions to the EATEN state and will be cleaned up by the
 * environment. Uses atomic state to allow safe concurrent reads during the
 * parallelizable reaction phase.
 */
class Food : public EnvironmentObject {
public:
    /** @brief Construct a Food object at the origin with default energy (500). */
    Food() : EnvironmentObject(0, 0) {}

    /**
     * @brief Construct food with a custom energy value.
     * @param energy The amount of energy this food provides when consumed.
     */
    explicit Food(int energy) : EnvironmentObject(0, 0), energy(energy) {}

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
     * @return Energy added to the consuming organism's lifespan.
     */
    int getEnergy() const { return energy; }

private:
    /// Atomic state ensures safe concurrent access from the parallel reaction phase.
    std::atomic<FoodState> state{FoodState::FRESH};
    int energy = 500;  ///< Energy granted to the organism that eats this food
};

#endif
