#ifndef FOOD_HPP
#define FOOD_HPP

#include "BaseEnvironmentObject.hpp"

enum class FoodState {
    FRESH,
    EATEN
};

class Food : public BaseEnvironmentObject {
public:
    Food(float x, float y) : BaseEnvironmentObject(x, y) {}
    bool canBeEaten() { return state == FoodState::FRESH; }
    void eaten() { state = FoodState::EATEN; }

private:
    FoodState state = FoodState::FRESH;
};

#endif
