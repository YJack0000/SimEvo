#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <index/SpatialIndex.hpp>
#include <index/SpatialObjectWrapper.hpp>
#include <string>

Environment::Environment(int width, int height, const std::string &type)
    : width(width), height(height) {
    if (type == std::string("optimize")) {
        spatialIndex = std::make_unique<OptimizedSpatialIndex>();
    } else {
        spatialIndex = std::make_unique<DefaultSpatialIndex>();
    }
}

void Environment::addOrganism(const std::shared_ptr<Organism> &organism, int x,
                              int y) {
    auto wrappedOrganism =
        std::make_shared<SpatialObjectWrapper<Organism>>(organism, x, y);
    spatialIndex->insert(wrappedOrganism);
}

void Environment::addFood(int x, int y) {
    auto food = std::make_shared<Food>();
    auto wrappedFood = std::make_shared<SpatialObjectWrapper<Food>>(food, x, y);
    spatialIndex->insert(wrappedFood);
}
