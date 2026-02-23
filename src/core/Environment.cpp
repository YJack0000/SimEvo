#include <boost/uuid/uuid_io.hpp>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <index/DefaultSpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <memory>
#include <thread>
#include <utils/profiler.hpp>
#include <vector>

Environment::Environment(int width, int height, std::string type, int numThreads)
    : width(width), height(height), type(type), numThreads(numThreads) {
    if (type == "default") {
        spatialIndex = std::make_unique<DefaultSpatialIndex<boost::uuids::uuid>>();
    } else if (type == "optimized") {
        unsigned long size = static_cast<unsigned long>(std::max(width, height));
        spatialIndex = std::make_unique<OptimizedSpatialIndex<boost::uuids::uuid>>(size);
    } else {
        throw std::invalid_argument("Invalid type");
    }
}

void Environment::checkBounds(float x, float y) const {
    if (x < 0.0f || x > static_cast<float>(width) || y < 0.0f || y > static_cast<float>(height)) {
        throw std::out_of_range("Coordinates are out of the allowed range.");
    }
}

void Environment::add(const std::shared_ptr<Organism>& organism, float x, float y) {
    checkBounds(x, y);
    auto id = organism->getId();
    organism->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, organism});
}

void Environment::add(const std::shared_ptr<Food>& food, float x, float y) {
    checkBounds(x, y);
    auto id = food->getId();
    food->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, food});
}

void Environment::remove(const std::shared_ptr<Organism>& organism) {
    if (objectsMapper.find(organism->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Organism not found in Environment.");
    }
    spatialIndex->remove(organism->getId());
    objectsMapper.erase(organism->getId());
}

void Environment::remove(const std::shared_ptr<Food>& food) {
    if (objectsMapper.find(food->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Food not found in Environment.");
    }
    spatialIndex->remove(food->getId());
    objectsMapper.erase(food->getId());
}

void Environment::reset() {
    spatialIndex->clear();
    objectsMapper.clear();
    deadOrganisms.clear();
    foodConsumption = 0;
}

void Environment::simulateIteration(int iterations,
                                    std::function<void(const Environment&)> on_each_iteration) {
    Profiler& profiler = Profiler::getInstance();
    profiler.reset();

    profiler.start("simulateIteration");
    for (int i = 0; i < iterations; i++) {
        if (getAllOrganisms().empty() && getAllFoods().empty()) {
            break;
        }

        profiler.start("handleInteractions");
        handleInteractions();
        profiler.stop("handleInteractions");

        profiler.start("handleReactions");
        handleReactions();
        profiler.stop("handleReactions");

        profiler.start("postIteration");
        postIteration();
        profiler.stop("postIteration");

        if (on_each_iteration) {
            on_each_iteration(*this);
        }
    }
    profiler.stop("simulateIteration");

    cleanUp();

    profiler.report("handleInteractions");
    profiler.report("handleReactions");
    profiler.report("postIteration");
    profiler.report("simulateIteration");
    printf("Index type: %s\n", type.c_str());
    printf("Number of threads: %d\n", numThreads);
    printf("Total food consumption: %lu\n", foodConsumption);
    printf("Total dead organisms: %lu\n", deadOrganisms.size());
    printf("Total organisms: %lu\n", getAllOrganisms().size());
    printf("_______________________________________________________\n");
}

void Environment::cleanUp() {
    std::vector<boost::uuids::uuid> toRemove;
    for (const auto& object : objectsMapper) {
        auto organism = std::dynamic_pointer_cast<Organism>(object.second);
        auto food = std::dynamic_pointer_cast<Food>(object.second);
        if (organism && !organism->isAlive()) {
            deadOrganisms.push_back(organism);
            toRemove.push_back(object.first);
        }
        if (food && !food->canBeEaten()) {
            foodConsumption += 1;
            toRemove.push_back(object.first);
        }
    }
    for (const auto& id : toRemove) {
        spatialIndex->remove(id);
        objectsMapper.erase(id);
    }
}

std::vector<std::shared_ptr<Organism>> Environment::getDeadOrganisms() const {
    return deadOrganisms;
}

unsigned long Environment::getFoodConsumptionInIteration() const { return foodConsumption; }

void Environment::postIteration() {
    for (auto& object : objectsMapper) {
        object.second->postIteration();
    }
    updatePositionsInSpatialIndex();
}

void Environment::updatePositionsInSpatialIndex() {
    for (auto& object : objectsMapper) {
        auto organism = std::dynamic_pointer_cast<Organism>(object.second);
        if (organism && organism->isAlive()) {
            auto [x, y] = organism->getPosition();

            x = std::max(0.0f, std::min(static_cast<float>(width), x));
            y = std::max(0.0f, std::min(static_cast<float>(height), y));

            organism->setPosition(x, y);
            spatialIndex->update(object.first, x, y);
        }
    }
}

// Interactions mutate shared state (food eaten, organism killed, lifeSpan changes),
// so this phase runs single-threaded to avoid data races.
void Environment::handleInteractions() {
    auto organisms = getAllOrganisms();

    for (auto& organism : organisms) {
        if (organism->isAlive()) {
            auto position = organism->getPosition();
            auto interactables =
                spatialIndex->query(position.first, position.second, organism->getSize());
            std::vector<std::shared_ptr<EnvironmentObject>> interactableObjects;

            for (auto& interactable : interactables) {
                if (interactable != organism->getId()) {
                    auto it = objectsMapper.find(interactable);
                    if (it != objectsMapper.end()) {
                        interactableObjects.push_back(it->second);
                    }
                }
            }

            organism->interact(interactableObjects);
        }
    }
}

// Reactions only write to each organism's own movement/reactionCounter fields,
// so this phase is safe to parallelize across organisms.
void Environment::handleReactions() {
    auto organisms = getAllOrganisms();
    if (organisms.empty()) return;

    auto worker = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            auto& organism = organisms[i];
            if (organism->isAlive()) {
                auto position = organism->getPosition();
                auto reactables = spatialIndex->query(
                    position.first, position.second, organism->getReactionRadius());
                std::vector<std::shared_ptr<EnvironmentObject>> reactableObjects;
                for (auto& reactable : reactables) {
                    if (reactable != organism->getId()) {
                        auto it = objectsMapper.find(reactable);
                        if (it != objectsMapper.end()) {
                            reactableObjects.push_back(it->second);
                        }
                    }
                }

                organism->react(reactableObjects);
            }
        }
    };

    if (numThreads <= 1) {
        worker(0, organisms.size());
    } else {
        std::vector<std::thread> threads;
        size_t chunkSize = organisms.size() / numThreads;

        for (int i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? organisms.size() : (i + 1) * chunkSize;
            threads.emplace_back(worker, start, end);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
}

std::vector<std::shared_ptr<EnvironmentObject>> Environment::getAllObjects() const {
    std::vector<std::shared_ptr<EnvironmentObject>> objects;
    for (const auto& object : objectsMapper) {
        objects.push_back(object.second);
    }
    return objects;
}

std::vector<std::shared_ptr<Organism>> Environment::getAllOrganisms() const {
    std::vector<std::shared_ptr<Organism>> organisms;
    for (const auto& object : objectsMapper) {
        if (auto organism = std::dynamic_pointer_cast<Organism>(object.second)) {
            organisms.push_back(organism);
        }
    }
    return organisms;
}

std::vector<std::shared_ptr<Food>> Environment::getAllFoods() const {
    std::vector<std::shared_ptr<Food>> foods;
    for (const auto& object : objectsMapper) {
        if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            foods.push_back(food);
        }
    }
    return foods;
}
