#include <boost/uuid/uuid_io.hpp>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <index/SpatialIndex.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

Environment::Environment(int width, int height, std::string type)
    : width(width), height(height) {
    if (type == "default") {
        spatialIndex =
            std::make_unique<DefaultSpatialIndex<boost::uuids::uuid>>();
    } else if (type == "optimized") {
        spatialIndex =
            std::make_unique<OptimizedSpatialIndex<boost::uuids::uuid>>(1000);
    } else {
        throw std::invalid_argument("Invalid type");
    }
}

void Environment::addOrganism(const std::shared_ptr<Organism>& organism,
                              float x, float y) {
    auto id = organism->getId();
    organism->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper[id] = organism;
}

void Environment::addFood(int x, int y) {
    auto food = std::make_shared<Food>();
    food->setPosition(x, y);
    auto id = food->getId();
    spatialIndex->insert(id, x, y);
    objectsMapper[id] = food;
}

void Environment::simulateIteration(
    int iterations, std::function<void(const Environment&)> on_each_iteration) {
    for (int i = 0; i < iterations; i++) {
        handleInteractions();
        postIteration();

        if (on_each_iteration) {
            on_each_iteration(*this);
        }
    }
}

void Environment::postIteration() {
    // do the post iteration for all objects
    for (auto& object : objectsMapper) {
        object.second->postIteration();
    }

    // remove dead organisms and food
    for (auto& object : objectsMapper) {
        if (auto organism =
                std::dynamic_pointer_cast<Organism>(object.second)) {
            if (!organism->isAlive()) {
                std::cout << "Removing dead organism: " << object.first
                          << std::endl;
                spatialIndex->remove(object.first);
                objectsMapper.erase(object.first);
            }
        } else if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            if(!food->canBeEaten()) {
                std::cout << "Removing eaten food: " << object.first << std::endl;
                spatialIndex->remove(object.first);
                objectsMapper.erase(object.first);
            }
        }
    }

    // update position of all objects
    for (auto& object : objectsMapper) {
        if (auto organism =
                std::dynamic_pointer_cast<Organism>(object.second)) {
            // move the alive organisms
            if (!organism->isAlive()) {
                continue;
            }

            // add boundary check
            std::pair<float, float> position = organism->getPosition();
            if(position.first < 0 || position.first > width) {
                position.first = std::max(0.0f, width < position.first ? width : position.first);
            }
            if(position.second < 0 || position.second > height) {
                position.second = std::max(0.0f, height < position.second ? height : position.second);
            }
            organism->setPosition(position.first, position.second);

            spatialIndex->update(object.first,
                                organism->getPosition().first,
                                organism->getPosition().second);
        }
    }
}

/**
 * @brief Handle interactions between objects
 *
 */
void Environment::handleInteractions() {
    for (auto& object : objectsMapper) {
        if (auto food =
                std::dynamic_pointer_cast<Food>(object.second)) {  // skip food
            continue;
        }

        if (auto organism =
                std::dynamic_pointer_cast<Organism>(object.second)) {
            if (!organism->isAlive()) {
                continue;
            }

            std::pair<float, float> position = organism->getPosition();

            // get all objects that are within the interaction radius, which is
            std::vector<boost::uuids::uuid> interactables =
                spatialIndex->query(position.first, position.second, 1);
            std::cout << "Interactables: " << interactables.size() << std::endl;
            for (auto& interactable : interactables) {
                if(interactable == object.first) { // skip itself
                    continue;
                }
                organism->interact(objectsMapper[interactable]);
            }

            // get objects that are within the react radius, which is defned by
            // genes
            std::vector<boost::uuids::uuid> reactables = spatialIndex->query(
                position.first, position.second, organism->getReactionRadius());
            std::cout << "Reactables: " << reactables.size() << std::endl;
            for (auto& reactable : reactables) {
                if (reactable == object.first) { // skip itself
                    continue;
                }
                // react with the object pointer from objectMapper
                organism->react(objectsMapper[reactable]);
            }
        }
    }
}

std::vector<std::shared_ptr<BaseEnvironmentObject>> Environment::getAllObjects()
    const {
    std::cout << "\n===================\n";
    std::cout << "Getting all objects" << std::endl;
    std::vector<std::shared_ptr<BaseEnvironmentObject>> objects;
    for (const auto& object : objectsMapper) {
        if (auto organism =
                std::dynamic_pointer_cast<Organism>(object.second)) {
            std::cout << std::left << std::setw(10)
                      << "Organism: " << std::setw(38) << object.first
                      << std::setw(4) << (int)object.second->getPosition().first
                      << std::setw(4)
                      << (int)object.second->getPosition().second << " "
                      << std::setw(6)
                      << (organism->isAlive() ? "Alive" : "Dead")
                      << std::setw(10) << organism->getSpeed() << std::setw(10)
                      << organism->getSize() << std::setw(10)
                      << organism->getAwareness() << std::endl;
        } else if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            std::cout << std::left << std::setw(10) << "Food: " << std::setw(38)
                      << object.first << std::setw(4)
                      << object.second->getPosition().first << std::setw(4)
                      << object.second->getPosition().second << " "
                      << (food->canBeEaten() ? "fresh" : "eaten") << std::endl;
        }
        objects.push_back(object.second);
    }
    return objects;
}
