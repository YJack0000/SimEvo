#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <index/SpatialIndex.hpp>
#include <memory>

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
    auto food = std::make_shared<Food>(x, y);
    food->setPosition(x, y);
    auto id = food->getId();
    spatialIndex->insert(id, x, y);
    objectsMapper[id] = food;
}

void Environment::simulateIteration(int iterations) {
    for (int i = 0; i < iterations; i++) {
        handleInteractions();
    }
    postIteration();
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
                spatialIndex->remove(object.first);
            }
        } else if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            if (!food->canBeEaten()) {
                spatialIndex->remove(object.first);
            }
        }
    }

    // update position of all organisms
    for (auto& object : objectsMapper) {
        spatialIndex->update(object.first, object.second->getPosition().first,
                             object.second->getPosition().second);
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

            // get all objects that are within the interaction radius, which is 0
            std::vector<boost::uuids::uuid> interactables =
                spatialIndex->query(position.first, position.second, 1);
            for (auto& interactable : interactables) {
                organism->interact(objectsMapper[interactable]);
            }

            // get objects that are within the react radius, which is defned by genes
            std::vector<boost::uuids::uuid> reactables = spatialIndex->query(
                position.first, position.second, organism->getReactionRadius());
            for (auto& reactable : reactables) {
                if (reactable == object.first) {
                    continue;
                }
                // react with the object pointer from objectMapper
                organism->react(objectsMapper[reactable]);
            }
        }
    }
}
