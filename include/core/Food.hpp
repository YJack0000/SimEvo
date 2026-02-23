#ifndef FOOD_HPP
#define FOOD_HPP

#include <atomic>

#include "EnvironmentObject.hpp"

enum class FoodState { FRESH, EATEN };

class Food : public EnvironmentObject {
public:
    Food() : EnvironmentObject(0, 0) {}
    bool canBeEaten() const { return state.load(std::memory_order_acquire) == FoodState::FRESH; }
    void eaten() { state.store(FoodState::EATEN, std::memory_order_release); }
    int getEnergy() const { return 500; }

private:
    std::atomic<FoodState> state{FoodState::FRESH};
};

#endif
