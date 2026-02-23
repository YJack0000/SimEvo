#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <random>

Organism::Organism() : EnvironmentObject(0, 0), genes("\x14\x14\x14\x14"), lifeSpan(500) {}

Organism::Organism(const Genes& genes) : EnvironmentObject(0, 0), genes(genes), lifeSpan(500) {}

Organism::Organism(const Genes& genes, LifeConsumptionCalculator calculator)
    : EnvironmentObject(0, 0), genes(genes), lifeConsumptionCalculator(calculator), lifeSpan(500) {}

/// DNA gene ranges: each byte 0-255 is mapped to 0-64 by dividing by 4.
float Organism::getSpeed() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(0))) / 4.0f;
}

float Organism::getSize() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(1))) / 4.0f;
}

float Organism::getAwareness() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(2))) / 4.0f;
}

/**
 * @brief Calculate life consumed per iteration based on organism attributes.
 * @return Life points consumed; higher speed/size/awareness costs more.
 *
 * Uses a quadratic formula so larger, faster organisms burn energy disproportionately.
 */
float Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

    // Quadratic cost: speed^2 + size^3 + awareness, scaled by 1.3
    float consumption =
        (getSpeed() / 10 * getSpeed() / 10 + getSize() / 10 * getSize() / 10 * getSize() / 15 +
        getAwareness() / 10) * 1.3;
    return consumption;
}

float Organism::getLifeSpan() const { return lifeSpan; }

float Organism::getReactionRadius() const { return getSize() + getAwareness(); }

bool Organism::isAlive() const { return lifeSpan > 0; }

bool Organism::canReproduce() const { return lifeSpan > 1000; }

double Organism::calculateDistance(const std::shared_ptr<EnvironmentObject>& object) const {
    auto pos = getPosition();
    auto otherPos = object->getPosition();
    auto dx = pos.first - otherPos.first;
    auto dy = pos.second - otherPos.second;
    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Decide movement direction based on the nearest relevant object.
 * @param reactableObjects Nearby objects within the organism's reaction radius.
 *
 * Behavior:
 * - Flee from organisms that are 1.5x larger (move away).
 * - Chase organisms that are 1.5x smaller (move toward to prey).
 * - Move toward the nearest edible food.
 * - Only one reaction per iteration (tracked by reactionCounter).
 *
 * Thread-safe: only writes to this organism's own movement/reactionCounter.
 */
void Organism::react(const std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {
    if (reactableObjects.empty()) {
        return;
    }

    // Find the nearest valid (alive/edible) object
    std::shared_ptr<EnvironmentObject> nearestObject = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& obj : reactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(obj)) {
            if (!food->canBeEaten()) {
                continue;
            }
        } else if (auto organism = std::dynamic_pointer_cast<Organism>(obj)) {
            if (!organism->isAlive()) {
                continue;
            }
        }

        double distance = calculateDistance(obj);
        if (distance < minDistance) {
            minDistance = distance;
            nearestObject = obj;
        }
    }

    if (!nearestObject) {
        return;
    }

    auto myPos = getPosition();

    if (auto otherOrganism = std::dynamic_pointer_cast<Organism>(nearestObject)) {
        // Only react to one organism per iteration
        if (reactionCounter != 0) {
            return;
        }

        auto otherPos = otherOrganism->getPosition();
        if (getSize() * 1.5 < otherOrganism->getSize()) {
            // Flee: move away from a much larger predator
            movement = Vec2(myPos.first - otherPos.first, myPos.second - otherPos.second);
            reactionCounter++;
        } else if (getSize() > 1.5 * otherOrganism->getSize()) {
            // Chase: move toward a much smaller prey
            movement = Vec2(otherPos.first - myPos.first, otherPos.second - myPos.second);
            reactionCounter++;
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(nearestObject)) {
        if (!food->canBeEaten()) {
            return;
        }

        reactionCounter++;
        // Move toward the food source
        auto foodPos = food->getPosition();
        movement = Vec2(foodPos.first - myPos.first, foodPos.second - myPos.second);
    } else {
        throw std::runtime_error("Unknown object type detected");
    }
}

/**
 * @brief Consume food and prey on smaller organisms within interaction range.
 * @param interactableObjects Objects overlapping with this organism's body.
 *
 * Eating food adds energy to lifespan. Killing a smaller organism (size > 1.5x)
 * absorbs its remaining lifespan. This method mutates shared state and must
 * run single-threaded.
 */
void Organism::interact(const std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {
    for (const auto& object : interactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(object)) {
            if (!food->canBeEaten()) continue;
            food->eaten();
            lifeSpan += food->getEnergy();
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
            // Predation: must be at least 1.5x larger to kill and absorb
            if (getSize() > 1.5 * organism->getSize() && organism->isAlive()) {
                lifeSpan += organism->getLifeSpan();
                organism->killed();
            }
        }
    }
}

/**
 * @brief Create an offspring with mutated genes near the parent.
 * @return Shared pointer to the newly created child organism.
 *
 * The child inherits the parent's genes with small mutations and the same
 * life consumption calculator. The parent's lifespan is halved as a cost.
 */
std::shared_ptr<Organism> Organism::reproduce() {
    Genes newGenes = genes;
    newGenes.mutate();
    auto newOrganism = std::make_shared<Organism>(newGenes, lifeConsumptionCalculator);
    // Offset child slightly from parent to avoid immediate overlap
    newOrganism->setPosition(getPosition().first + 2, getPosition().second + 2);
    lifeSpan /= 2;
    return newOrganism;
}

void Organism::killed() { lifeSpan = 0; }

void Organism::postIteration() {
    lifeSpan -= getLifeConsumption();

    if (lifeSpan <= 0) {
        killed();
        return;
    }

    makeMove();
}

/**
 * @brief Execute movement for this iteration.
 *
 * If no reaction occurred, the organism has an 80% chance to keep its
 * current movement direction (dis(gen) > 0 yields 4/5 probability).
 * Movement is then normalized to the organism's speed.
 * Uses thread_local RNG for thread safety during parallel execution.
 */
void Organism::makeMove() {
    // thread_local ensures each thread gets its own RNG for safe parallel calls
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(0, 4);
    std::uniform_int_distribution<int> move(-1, 1);

    auto speed = getSpeed();

    if (reactionCounter == 0) {
        // 80% chance to keep current movement direction (4 out of 5 outcomes)
        bool keepMovement = dis(gen) > 0;
        if (movement.isZero()) {
            keepMovement = false;
        }

        if (!keepMovement) {
            movement = Vec2(move(gen) * speed, move(gen) * speed);
        }
    }

    // Normalize movement vector to organism's speed
    movement = movement.normalized(speed);

    setPosition(getPosition().first + movement.x, getPosition().second + movement.y);

    reactionCounter = 0;
}
