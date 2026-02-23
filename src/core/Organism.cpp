#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <cstdio>
#include <random>

Organism::Organism() : EnvironmentObject(0, 0), genes("\x14\x14\x14\x14"), lifeSpan(500) {}

Organism::Organism(const Genes& genes) : EnvironmentObject(0, 0), genes(genes), lifeSpan(500) {}

Organism::Organism(const Genes& genes, LifeConsumptionCalculator calculator)
    : EnvironmentObject(0, 0), genes(genes), lifeConsumptionCalculator(calculator), lifeSpan(500) {}

float Organism::getSpeed() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(0))) / 4.0f;
}

float Organism::getSize() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(1))) / 4.0f;
}

float Organism::getAwareness() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(2))) / 4.0f;
}

float Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

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

void Organism::react(const std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {
    if (reactableObjects.empty()) {
        return;
    }

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
        if (reactionCounter != 0) {
            return;
        }

        auto otherPos = otherOrganism->getPosition();
        if (getSize() * 1.5 < otherOrganism->getSize()) {
            movement = std::make_pair(myPos.first - otherPos.first,
                                      myPos.second - otherPos.second);
            reactionCounter++;
        } else if (getSize() > 1.5 * otherOrganism->getSize()) {
            movement = std::make_pair(otherPos.first - myPos.first,
                                      otherPos.second - myPos.second);
            reactionCounter++;
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(nearestObject)) {
        if (!food->canBeEaten()) {
            return;
        }

        reactionCounter++;
        auto foodPos = food->getPosition();
        movement = std::make_pair(foodPos.first - myPos.first,
                                  foodPos.second - myPos.second);
    } else {
        throw std::runtime_error("Unknown object type detected");
    }
}

void Organism::interact(const std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {
    for (const auto& object : interactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(object)) {
            if (!food->canBeEaten()) continue;
            food->eaten();
            lifeSpan += food->getEnergy();
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
            if (getSize() > 1.5 * organism->getSize() && organism->isAlive()) {
                lifeSpan += organism->getLifeSpan();
                organism->killed();
            }
        }
    }
}

std::shared_ptr<Organism> Organism::reproduce() {
    Genes newGenes = genes;
    newGenes.mutate();
    auto newOrganism = std::make_shared<Organism>(newGenes, lifeConsumptionCalculator);
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

void Organism::makeMove() {
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dis(0, 4);
    std::uniform_int_distribution<int> move(-1, 1);

    auto speed = getSpeed();

    if (reactionCounter == 0) {
        bool keepMovement = dis(gen) > 0;
        if (movement.first == 0 && movement.second == 0) {
            keepMovement = false;
        }

        if (!keepMovement) {
            movement = std::make_pair(move(gen) * speed, move(gen) * speed);
        }
    }

    auto movementLength =
        std::sqrt(movement.first * movement.first + movement.second * movement.second);
    if (movementLength > speed) {
        double scalingFactor = speed / movementLength;
        movement.first *= scalingFactor;
        movement.second *= scalingFactor;
    }

    setPosition(getPosition().first + movement.first, getPosition().second + movement.second);

    reactionCounter = 0;
}
