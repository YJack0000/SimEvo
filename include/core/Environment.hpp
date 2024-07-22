// Environment.hpp
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <memory>
#include <unordered_map>

#include "Food.hpp"
#include "Organism.hpp"
#include "index/ISpatialIndex.hpp"

class Environment {
public:
    Environment(int width, int height, std::string type = "default", int numThreads = 1);
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void add(const std::shared_ptr<Organism>& organism, float x, float y);
    void add(const std::shared_ptr<Food>& food, float x, float y);

    void remove(const std::shared_ptr<Organism>& organism);
    void remove(const std::shared_ptr<Food>& food);

    void reset();

    void simulateIteration(int,
                           std::function<void(const Environment&)> on_each_iteration = nullptr);

    std::vector<std::shared_ptr<Organism>> getAllOrganisms() const;
    std::vector<std::shared_ptr<Food>> getAllFoods() const;
    std::vector<std::shared_ptr<EnvironmentObject>> getAllObjects() const;
    std::vector<std::shared_ptr<Organism>> getDeadOrganisms() const;
    unsigned long getFoodConsumptionInIteration() const;

private:
    int width, height;
    std::string type;
    std::unique_ptr<ISpatialIndex<boost::uuids::uuid>> spatialIndex;
    std::unordered_map<boost::uuids::uuid, std::shared_ptr<EnvironmentObject>> objectsMapper;

    std::vector<std::shared_ptr<Organism>> deadOrganisms;
    unsigned long foodConsumption;

    int numThreads = 1;

    void checkBounds(float x, float y) const;
    void updatePositionsInSpatialIndex();
    void handleInteractions();
    void handleReactions();
    void postIteration();
    void cleanUp();
    void removeDeadOrganisms();
};

#endif
