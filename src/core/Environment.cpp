#include <algorithm>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <index/SpatialIndex.hpp>
#include <index/SpatialObjectWrapper.hpp>
#include <string>

Environment::Environment(int width, int height, const std::string &type)
    : width(width), height(height) {
    if (type == std::string("optimize")) {
        int size = std::max(width, height);
        spatialIndex = std::make_unique<OptimizedSpatialIndex>(size);
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

void Environment::simulateIteration(int iterations) {
    for (int i = 0; i < iterations; i++) {
        handleInteractions();
        removeDeadOrganisms();
        spawnOrganisms();
    }
}

void Environment::handleInteractions() {
    for (const auto &object : *spatialIndex) {
        if (auto organism =
                std::dynamic_pointer_cast<Organism>(object->getObject())) {
            auto neighbors = spatialIndex->query(object->getPosition().first,
                                                 object->getPosition().second,
                                                 organism->getAwareness());

            for (const auto &neighbor : neighbors) {
                if (auto other_organism = std::dynamic_pointer_cast<Organism>(
                        neighbor->getObject())) {
                    organism->interact(other_organism);
                }
            }
        } else if (auto food =
                       std::dynamic_pointer_cast<Food>(object->getObject())) {
            // do nothing
        }
    }
}
