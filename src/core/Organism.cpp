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

void Organism::addLifeSpan(float amount) { lifeSpan += amount; }

void Organism::setReactionStrategy(ReactionStrategy strategy) {
    reactionStrategy = std::move(strategy);
}

void Organism::setInteractionStrategy(InteractionStrategy strategy) {
    interactionStrategy = std::move(strategy);
}

bool Organism::hasCustomStrategy() const {
    return static_cast<bool>(reactionStrategy) || static_cast<bool>(interactionStrategy);
}

double Organism::calculateDistance(const std::shared_ptr<EnvironmentObject>& object) const {
    auto pos = getPosition();
    auto otherPos = object->getPosition();
    auto dx = pos.first - otherPos.first;
    auto dy = pos.second - otherPos.second;
    return std::sqrt(dx * dx + dy * dy);
}

// --- Default built-in strategies ---

/**
 * @brief Built-in reaction: find nearest valid object and decide movement.
 *
 * Movement rules:
 *  - Flee from organisms whose size exceeds 1.5x this organism's size.
 *  - Chase organisms whose size is less than 2/3 of this organism's size.
 *  - Move toward edible food.
 *  - Return {0,0} when no actionable object is found.
 */
std::pair<float, float> Organism::defaultReaction(
    Organism& self,
    const std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {

    std::shared_ptr<EnvironmentObject> nearestObject = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& obj : reactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(obj)) {
            if (!food->canBeEaten()) continue;
        } else if (auto organism = std::dynamic_pointer_cast<Organism>(obj)) {
            if (!organism->isAlive()) continue;
        }

        double distance = self.calculateDistance(obj);
        if (distance < minDistance) {
            minDistance = distance;
            nearestObject = obj;
        }
    }

    if (!nearestObject) {
        return {0.0f, 0.0f};
    }

    auto myPos = self.getPosition();

    if (auto otherOrganism = std::dynamic_pointer_cast<Organism>(nearestObject)) {
        auto otherPos = otherOrganism->getPosition();
        if (self.getSize() * 1.5 < otherOrganism->getSize()) {
            return {myPos.first - otherPos.first, myPos.second - otherPos.second};
        } else if (self.getSize() > 1.5 * otherOrganism->getSize()) {
            return {otherPos.first - myPos.first, otherPos.second - myPos.second};
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(nearestObject)) {
        if (food->canBeEaten()) {
            auto foodPos = food->getPosition();
            return {foodPos.first - myPos.first, foodPos.second - myPos.second};
        }
    }

    return {0.0f, 0.0f};
}

/**
 * @brief Built-in interaction: eat food and prey on smaller organisms.
 *
 * For each nearby object:
 *  - Edible food is consumed, adding its energy to life-span.
 *  - Organisms smaller than 2/3 of self's size are killed and their remaining
 *    life-span is absorbed.
 */
void Organism::defaultInteraction(
    Organism& self,
    const std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {

    for (const auto& object : interactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(object)) {
            if (!food->canBeEaten()) continue;
            self.addLifeSpan(food->getEnergy());
            food->eaten();
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
            if (self.getSize() > 1.5 * organism->getSize() && organism->isAlive()) {
                self.addLifeSpan(organism->getLifeSpan());
                organism->killed();
            }
        }
    }
}

// --- Public action methods ---

void Organism::react(const std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {
    if (reactableObjects.empty()) return;
    if (reactionCounter != 0) return;

    std::pair<float, float> result;
    if (reactionStrategy) {
        result = reactionStrategy(*this, reactableObjects);
    } else {
        result = defaultReaction(*this, reactableObjects);
    }

    if (result.first != 0.0f || result.second != 0.0f) {
        movement = Vec2(result.first, result.second);
        reactionCounter++;
    }
}

void Organism::interact(const std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {
    if (interactionStrategy) {
        interactionStrategy(*this, interactableObjects);
    } else {
        defaultInteraction(*this, interactableObjects);
    }
}

/**
 * @brief Create a mutated offspring and halve this organism's life-span.
 *
 * The child inherits the parent's genes (with mutation), life-consumption
 * calculator, and any custom reaction/interaction strategies.
 */
std::shared_ptr<Organism> Organism::reproduce() {
    Genes newGenes = genes;
    newGenes.mutate();
    auto newOrganism = std::make_shared<Organism>(newGenes, lifeConsumptionCalculator);
    if (reactionStrategy) newOrganism->setReactionStrategy(reactionStrategy);
    if (interactionStrategy) newOrganism->setInteractionStrategy(interactionStrategy);
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
 * current movement direction. Movement is then normalized to the organism's speed.
 * Uses thread_local RNG for thread safety during parallel execution.
 */
void Organism::makeMove() {
    // thread_local ensures each thread gets its own RNG for safe parallel calls
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(0, 4);
    std::uniform_int_distribution<int> move(-1, 1);

    auto speed = getSpeed();

    if (reactionCounter == 0) {
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
