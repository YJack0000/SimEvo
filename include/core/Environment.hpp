// Environment.hpp
#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Organism.hpp"
#include "index/ISpatialIndex.hpp"

class Environment {
public:
    Environment(int width, int height, std::string type = "default");

    void addOrganism(const std::shared_ptr<Organism>& organism, float x,
                     float y);
    void addFood(int x, int y);
    void simulateIteration(int);
    void postIteration();
    std::vector<std::shared_ptr<BaseEnvironmentObject>> getAllObjects() const;

private:
    int width, height;
    std::unique_ptr<ISpatialIndex<boost::uuids::uuid>> spatialIndex;
    std::unordered_map<boost::uuids::uuid,
                       std::shared_ptr<BaseEnvironmentObject>>
        objectsMapper;

    void handleInteractions();
    void removeDeadOrganisms();
    void spawnOrganisms();
};

#endif
