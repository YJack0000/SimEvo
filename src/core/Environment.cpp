#include <boost/uuid/uuid_io.hpp>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <index/DefaultSpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <memory>
#include <thread>
#include <utils/profiler.hpp>
#include <vector>

/**
 * @brief Construct an environment with the given dimensions and spatial index type.
 * @param width  Environment width in simulation units.
 * @param height Environment height in simulation units.
 * @param type   Spatial index type: "default" or "optimized".
 * @param numThreads Number of threads for the parallel reaction phase.
 * @throws std::invalid_argument If the spatial index type is unknown.
 */
Environment::Environment(int width, int height, std::string type, int numThreads)
    : width(width), height(height), type(type), numThreads(numThreads) {
    if (type == "default") {
        spatialIndex = std::make_unique<DefaultSpatialIndex<boost::uuids::uuid>>();
    } else if (type == "optimized") {
        // Use the longest side as the quadtree grid dimension
        unsigned long size = static_cast<unsigned long>(std::max(width, height));
        spatialIndex = std::make_unique<OptimizedSpatialIndex<boost::uuids::uuid>>(size);
    } else {
        throw std::invalid_argument("Invalid type");
    }
}

/**
 * @brief Validate that coordinates are within environment boundaries.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @throws std::out_of_range If coordinates exceed [0, width] x [0, height].
 */
void Environment::checkBounds(float x, float y) const {
    if (x < 0.0f || x > static_cast<float>(width) || y < 0.0f || y > static_cast<float>(height)) {
        throw std::out_of_range("Coordinates are out of the allowed range.");
    }
}

/**
 * @brief Add an organism to the environment at the specified position.
 * @param organism Shared pointer to the organism.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @throws std::out_of_range If coordinates are out of bounds.
 */
void Environment::add(const std::shared_ptr<Organism>& organism, float x, float y) {
    checkBounds(x, y);
    auto id = organism->getId();
    organism->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, organism});
}

/**
 * @brief Add a food item to the environment at the specified position.
 * @param food Shared pointer to the food.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @throws std::out_of_range If coordinates are out of bounds.
 */
void Environment::add(const std::shared_ptr<Food>& food, float x, float y) {
    checkBounds(x, y);
    auto id = food->getId();
    food->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, food});
}

/**
 * @brief Remove an organism from the environment.
 * @param organism Shared pointer to the organism to remove.
 * @throws std::runtime_error If the organism is not found.
 */
void Environment::remove(const std::shared_ptr<Organism>& organism) {
    if (objectsMapper.find(organism->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Organism not found in Environment.");
    }
    spatialIndex->remove(organism->getId());
    objectsMapper.erase(organism->getId());
}

/**
 * @brief Remove a food item from the environment.
 * @param food Shared pointer to the food to remove.
 * @throws std::runtime_error If the food is not found.
 */
void Environment::remove(const std::shared_ptr<Food>& food) {
    if (objectsMapper.find(food->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Food not found in Environment.");
    }
    spatialIndex->remove(food->getId());
    objectsMapper.erase(food->getId());
}

/**
 * @brief Reset the environment, removing all objects and clearing statistics.
 */
void Environment::reset() {
    spatialIndex->clear();
    objectsMapper.clear();
    deadOrganisms.clear();
    foodConsumption = 0;
}

/**
 * @brief Run the simulation for the specified number of iterations.
 * @param iterations Number of iterations to simulate.
 * @param on_each_iteration Optional callback invoked after each iteration.
 *
 * Each iteration runs three phases in order:
 * 1. handleInteractions (single-threaded) -- organisms eat food / prey on others.
 * 2. handleReactions (multi-threaded) -- organisms decide movement direction.
 * 3. postIteration -- deduct life, move organisms, sync spatial index.
 */
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

    if (verbose) {
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
}

/**
 * @brief Remove dead organisms and consumed food from the environment.
 *
 * Dead organisms are archived in deadOrganisms for post-simulation analysis.
 * Consumed food increments the foodConsumption counter. Uses deferred removal
 * to avoid invalidating the iterator during traversal.
 */
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

/** @brief Get the list of organisms that died during the simulation. */
std::vector<std::shared_ptr<Organism>> Environment::getDeadOrganisms() const {
    return deadOrganisms;
}

/** @brief Get the total number of food items consumed across all iterations. */
unsigned long Environment::getFoodConsumptionInIteration() const { return foodConsumption; }

/**
 * @brief Run per-object post-iteration logic, then sync positions with the spatial index.
 */
void Environment::postIteration() {
    for (auto& object : objectsMapper) {
        object.second->postIteration();
    }
    updatePositionsInSpatialIndex();
}

/**
 * @brief Synchronize organism positions with the spatial index after movement.
 *
 * Clamps organism positions within environment bounds before updating the index.
 */
void Environment::updatePositionsInSpatialIndex() {
    for (auto& object : objectsMapper) {
        auto organism = std::dynamic_pointer_cast<Organism>(object.second);
        if (organism && organism->isAlive()) {
            auto [x, y] = organism->getPosition();

            // Clamp organism position within environment bounds
            x = std::max(0.0f, std::min(static_cast<float>(width), x));
            y = std::max(0.0f, std::min(static_cast<float>(height), y));

            organism->setPosition(x, y);
            spatialIndex->update(object.first, x, y);
        }
    }
}

/**
 * @brief Run the interaction phase: organisms eat food and fight.
 *
 * Interactions mutate shared state (food eaten, organism killed, lifeSpan changes),
 * so this phase runs single-threaded to avoid data races.
 */
void Environment::handleInteractions() {
    auto organisms = getAllOrganisms();

    for (auto& organism : organisms) {
        if (organism->isAlive()) {
            auto position = organism->getPosition();
            // Query spatial index for objects within this organism's body size radius
            auto interactables =
                spatialIndex->query(position.first, position.second, organism->getSize());
            std::vector<std::shared_ptr<EnvironmentObject>> interactableObjects;

            for (auto& interactable : interactables) {
                // Exclude self from interaction targets
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

/**
 * @brief Run the reaction phase: organisms decide movement direction.
 *
 * Reactions only write to each organism's own movement/reactionCounter fields,
 * so this phase is safe to parallelize across organisms.
 */
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
        // Partition organisms evenly across threads; last thread handles remainder
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

/** @brief Get all objects (organisms and food) in the environment. */
std::vector<std::shared_ptr<EnvironmentObject>> Environment::getAllObjects() const {
    std::vector<std::shared_ptr<EnvironmentObject>> objects;
    for (const auto& object : objectsMapper) {
        objects.push_back(object.second);
    }
    return objects;
}

/** @brief Get all living and dead organisms currently in the environment. */
std::vector<std::shared_ptr<Organism>> Environment::getAllOrganisms() const {
    std::vector<std::shared_ptr<Organism>> organisms;
    for (const auto& object : objectsMapper) {
        if (auto organism = std::dynamic_pointer_cast<Organism>(object.second)) {
            organisms.push_back(organism);
        }
    }
    return organisms;
}

/** @brief Get all food items currently in the environment. */
std::vector<std::shared_ptr<Food>> Environment::getAllFoods() const {
    std::vector<std::shared_ptr<Food>> foods;
    for (const auto& object : objectsMapper) {
        if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            foods.push_back(food);
        }
    }
    return foods;
}
