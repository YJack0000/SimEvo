#ifndef FOOD_HPP
#define FOOD_HPP

#include "EnvironmentObject.hpp"

enum class FoodState { FRESH, EATEN };

class Food : public EnvironmentObject {
public:
    Food() : EnvironmentObject(0, 0) {}
    bool canBeEaten() { return state == FoodState::FRESH; }
    void eaten() { state = FoodState::EATEN; }
    int getEnergy() const { return 500; }

private:
    FoodState state = FoodState::FRESH;
};

#endif
