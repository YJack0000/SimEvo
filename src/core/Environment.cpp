#include <boost/uuid/uuid_io.hpp>
#include <core/Environment.hpp>
#include <core/Food.hpp>
#include <index/DefaultSpatialIndex.hpp>
#include <index/OptimizedSpatialIndex.hpp>
#include <memory>
#include <vector>
#include <chrono>

/**
 * @brief Constructor for Environment class.
 *
 * Initializes the Environment with given dimensions and a specific type of
 * spatial index.
 *
 * @param width The width of the environment.
 * @param height The height of the environment.
 * @param type The type of spatial index to use ("default" or "optimized").
 * @throws std::invalid_argument If an unrecognized type is provided.
 */
Environment::Environment(int width, int height, std::string type)
    : width(width), height(height), type(type) {
    if (type == "default") {
        spatialIndex = std::make_unique<DefaultSpatialIndex<boost::uuids::uuid>>();
    } else if (type == "optimized") {
        // setting the size of the spatial index to the longest side of the environment
        unsigned long size = static_cast<unsigned long>(std::max(width, height));
        spatialIndex = std::make_unique<OptimizedSpatialIndex<boost::uuids::uuid>>(size);
    } else {
        throw std::invalid_argument("Invalid type");
    }
}

/**
 * @brief Checks if the given coordinates are within the environment boundaries.
 *
 * Throws an exception if the coordinates are out of the allowed range.
 *
 * @param x The x-coordinate to check.
 * @param y The y-coordinate to check.
 * @throws std::out_of_range If coordinates are out of the environment
 * boundaries.
 */
void Environment::checkBounds(float x, float y) const {
    if (x < 0.0f || x > static_cast<float>(width) || y < 0.0f || y > static_cast<float>(height)) {
        // printf("Coordinates are out of the allowed range: (%f, %f)\n", x, y);
        throw std::out_of_range("Coordinates are out of the allowed range.");
    }
}

/**
 * @brief Adds an organism to the environment at specified coordinates.
 *
 * Inserts the organism into the spatial index and the objects map.
 *
 * @param organism A shared pointer to the organism to add.
 * @param x The x-coordinate where the organism will be placed.
 * @param y The y-coordinate where the organism will be placed.
 */
void Environment::add(const std::shared_ptr<Organism>& organism, float x, float y) {
    checkBounds(x, y);
    auto id = organism->getId();
    organism->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, organism});
}

/**
 * brief Adds a food item to the environment at specified coordinates.
 *
 * Inserts the food item into the spatial index and the objects map.
 *
 * @param food A shared pointer to the food item to add.
 * @param x The x-coordinate where the food item will be placed.
 * @param y The y-coordinate where the food item will be placed.
 */
void Environment::add(const std::shared_ptr<Food>& food, float x, float y) {
    checkBounds(x, y);
    auto id = food->getId();
    food->setPosition(x, y);
    spatialIndex->insert(id, x, y);
    objectsMapper.insert({id, food});
}

/**
 * @brief Removes an organism from the environment.
 *
 * Removes the organism from the spatial index and the objects map.
 *
 * @param organism A shared pointer to the organism to remove.
 */
void Environment::remove(const std::shared_ptr<Organism>& organism) {
    if (objectsMapper.find(organism->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Organism not found in Environment.");
    }

    spatialIndex->remove(organism->getId());
    objectsMapper.erase(organism->getId());
}

/**
 * brief Removes a food item from the environment.
 *
 * Removes the food item from the spatial index and the objects map.
 *
 * @param food A shared pointer to the food item to remove.
 */
void Environment::remove(const std::shared_ptr<Food>& food) {
    if (objectsMapper.find(food->getId()) == objectsMapper.end()) {
        throw std::runtime_error("Food not found in Environment.");
    }

    spatialIndex->remove(food->getId());
    objectsMapper.erase(food->getId());
}

/**
 * @brief Resets the environment by clearing the spatial index and object map.
 */
void Environment::reset() {
    spatialIndex->clear();
    objectsMapper.clear();
    deadOrganisms.clear();
    foodConsumption = 0;
}

/**
 * @brief Simulates a number of iterations of the environment.
 *
 * Each iteration includes handling interactions, a post-iteration update, and
 * calling a callback function.
 *
 * @param iterations Number of iterations to simulate.
 * @param on_each_iteration Callback function to be called after each iteration.
 */
void Environment::simulateIteration(int iterations,
                                    std::function<void(const Environment&)> on_each_iteration) {
    std::chrono::duration<double, std::milli> total_interactions_duration(0);
    std::chrono::duration<double, std::milli> total_post_iteration_duration(0);

    for (int i = 0; i < iterations; i++) {
        if (getAllOrganisms().size() == 0 && getAllFoods().size() == 0) {
            break;
        }

        auto interactions_start = std::chrono::high_resolution_clock::now();
        handleInteractions();
        auto interactions_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> interactions_duration =
            interactions_end - interactions_start;
        total_interactions_duration += interactions_duration;

        auto post_iteration_start = std::chrono::high_resolution_clock::now();
        postIteration();
        auto post_iteration_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> post_iteration_duration =
            post_iteration_end - post_iteration_start;
        total_post_iteration_duration += post_iteration_duration;

        if (on_each_iteration) {
            on_each_iteration(*this);
        }
    }

    // clean up dead organisms and consumed food
    cleanUp();

    if (iterations > 0) {
        double average_interactions_duration = total_interactions_duration.count() / iterations;
        double average_post_iteration_duration = total_post_iteration_duration.count() / iterations;

        printf("By %s spatial index\n", type.c_str());
        printf("Average handleInteractions duration: %f ms\n", average_interactions_duration);
        printf("Average postIteration duration: %f ms\n", average_post_iteration_duration);
        printf("Total time: %f ms, average time per iteration: %f ms\n",
               total_interactions_duration.count() + total_post_iteration_duration.count(),
               (total_interactions_duration.count() + total_post_iteration_duration.count()) /
                   iterations);
    }
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

/**
 * @brief Updates and cleans up the environment after each iteration.
 *
 * Removes dead organisms and consumed food, and updates the positions of all
 * active organisms.
 */
void Environment::postIteration() {
    // do the post iteration for all objects
    for (auto& object : objectsMapper) {
        object.second->postIteration();
    }

    // printf("Updating positions in spatial index\n");
    updatePositionsInSpatialIndex();
}

/**
 * @brief Updates the positions of all organisms in the spatial index, ensuring
 * they remain within bounds.
 */
void Environment::updatePositionsInSpatialIndex() {
    for (auto& object : objectsMapper) {
        auto organism = std::dynamic_pointer_cast<Organism>(object.second);
        if (organism && organism->isAlive()) {
            auto [x, y] = organism->getPosition();

            // make organism stay within bounds
            x = std::max(0.0f, std::min(static_cast<float>(width), x));
            y = std::max(0.0f, std::min(static_cast<float>(height), y));

            organism->setPosition(x, y);
            spatialIndex->update(object.first, x, y);
        }
    }
}

/**
 * @brief Manages interactions between all living organisms within the
 * environment.
 *
 */
void Environment::handleInteractions() {
    for (auto& object : objectsMapper) {
        if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {  // skip food
            continue;
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object.second)) {
            if (!organism->isAlive()) {
                continue;
            }

            std::pair<float, float> position = organism->getPosition();

            // printf("------------Handling interactions----------------\n");
            // get all objects that are within the interaction radius, which is
            std::vector<boost::uuids::uuid> interactables =
                spatialIndex->query(position.first, position.second, organism->getSize());
            std::vector<std::shared_ptr<EnvironmentObject>> interactableObjects;
            for (auto& interactable : interactables) {
                if (interactable == object.first) {  // skip itself
                    continue;
                }
                interactableObjects.push_back(objectsMapper[interactable]);
            }
            // printf("Interactables: %ld\n", interactableObjects.size());
            organism->interact(interactableObjects);

            // printf("------------Handling reactions----------------\n");
            // get objects that are within the react radius, which is defned by genes
            std::vector<boost::uuids::uuid> reactables =
                spatialIndex->query(position.first, position.second, organism->getReactionRadius());
            std::vector<std::shared_ptr<EnvironmentObject>> reactableObjects;
            for (auto& reactable : reactables) {
                if (reactable == object.first) {  // skip itself
                    continue;
                }
                reactableObjects.push_back(objectsMapper[reactable]);
            }
            // printf("Reactables: %ld\n", reactableObjects.size());
            organism->react(reactableObjects);
        }
    }
}

/** @brief Retrieves all objects currently in the environment.
 *
 * @return std::vector<std::shared_ptr<EnvironmentObject>> A list of all objects.
 */
std::vector<std::shared_ptr<EnvironmentObject>> Environment::getAllObjects() const {
    std::vector<std::shared_ptr<EnvironmentObject>> objects;
    for (const auto& object : objectsMapper) {
        objects.push_back(object.second);
    }
    return objects;
}

/**
 * @brief Retrieves all organisms currently in the environment.
 *
 * @return std::vector<std::shared_ptr<Organism>> A list of all organisms.
 */
std::vector<std::shared_ptr<Organism>> Environment::getAllOrganisms() const {
    std::vector<std::shared_ptr<Organism>> organisms;
    for (const auto& object : objectsMapper) {
        if (auto organism = std::dynamic_pointer_cast<Organism>(object.second)) {
            organisms.push_back(organism);
        }
    }
    return organisms;
}

/**
 * @brief Retrieves all food items currently in the environment.
 *
 * @return std::vector<std::shared_ptr<Food>> A list of all food items.
 */
std::vector<std::shared_ptr<Food>> Environment::getAllFoods() const {
    std::vector<std::shared_ptr<Food>> foods;
    for (const auto& object : objectsMapper) {
        if (auto food = std::dynamic_pointer_cast<Food>(object.second)) {
            foods.push_back(food);
        }
    }
    return foods;
}
