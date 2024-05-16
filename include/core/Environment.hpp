// Environment.hpp
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <memory>

#include "index/ISpatialIndex.hpp"
#include "Organism.hpp"

class Environment {
public:
    Environment(int width, int height, const std::string& type);

    void addOrganism(const std::shared_ptr<Organism>& organism, int x, int y);
    void addFood(int x, int y);
    void simulateIteration(int);
    void postIteration();

private:
    int width, height;
    std::unique_ptr<ISpatialIndex> spatialIndex;

    void handleInteractions();
    void removeDeadOrganisms();
    void spawnOrganisms();
};

#endif
