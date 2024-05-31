#include <boost/uuid/uuid_io.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <iostream>
#include <random>

Organism::Organism() : EnvironmentObject(0, 0), genes("\x14\x14\x14\x14") {}

Organism::Organism(const Genes& genes) : EnvironmentObject(0, 0), genes(genes) {}

Organism::Organism(const Genes& genes, LifeConsumptionCalculator calculator)
    : EnvironmentObject(0, 0), genes(genes), lifeConsumptionCalculator(calculator) {}

float Organism::getSpeed() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(0))) /
           20.0f;  // 256 / 20 = 12.8= 12.8
}

float Organism::getSize() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(1))) /
           40.0f;  // 256 / 40 = 6.4
}

float Organism::getAwareness() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(2))) /
           20.0f;  // 256 / 20 = 12.8
}

float Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

    printf("Organism %s is getting speed %f, size %f, awareness %f\n",
           boost::uuids::to_string(getId()).c_str(), getSpeed(), getSize(), getAwareness());
    return getSpeed() * getSize() * getAwareness() / 10;
}

float Organism::getLifeSpan() const { return lifeSpan; }

float Organism::getReactionRadius() const { return getAwareness(); }

bool Organism::isAlive() const { return lifeSpan > 0; }

bool Organism::canReproduce() const { return lifeSpan > 300; }

double Organism::calculateDistance(std::shared_ptr<EnvironmentObject> object) {
    auto dx = getPosition().first - object->getPosition().first;
    auto dy = getPosition().second - object->getPosition().second;
    return std::sqrt(dx * dx + dy * dy);
}

void Organism::react(std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {
    if (reactableObjects.empty()) {
        printf("Organism %s is not reacting to any objects\n",
               boost::uuids::to_string(getId()).c_str());
        return;
    }
    printf("Organism %s is reacting to the other objects\n",
           boost::uuids::to_string(getId()).c_str());

    std::shared_ptr<EnvironmentObject> nearestObject = reactableObjects[0];
    double minDistance = calculateDistance(nearestObject);

    printf("Organism %s is calculating the nearest object\n",
           boost::uuids::to_string(getId()).c_str());

    for (size_t i = 1; i < reactableObjects.size(); ++i) {
        double distance = calculateDistance(reactableObjects[i]);
        if (distance < minDistance) {
            minDistance = distance;
            nearestObject = reactableObjects[i];
        }
    }

    if (auto otherOrganism = std::dynamic_pointer_cast<Organism>(nearestObject)) {
        if (reactionCounter != 0) {  // already decided to move
            return;
        }
        reactionCounter++;

        if (getSize() * 1.5 < otherOrganism->getSize()) {
            printf("Organism %s is running away from organism %s\n",
                   boost::uuids::to_string(getId()).c_str(),
                   boost::uuids::to_string(otherOrganism->getId()).c_str());
            // run away from the other organism
            movement = std::make_pair(getPosition().first - otherOrganism->getPosition().first,
                                      getPosition().second - otherOrganism->getPosition().second);
        } else if (getSize() > 1.5 * otherOrganism->getSize()) {
            printf("Organism %s is chasing organism %s\n", boost::uuids::to_string(getId()).c_str(),
                   boost::uuids::to_string(otherOrganism->getId()).c_str());
            // trace the other organism
            movement = std::make_pair(otherOrganism->getPosition().first - getPosition().first,
                                      otherOrganism->getPosition().second - getPosition().second);
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(nearestObject)) {
        if (!food->canBeEaten()) return;
        printf("Organism %s is moving to the food \n", boost::uuids::to_string(getId()).c_str());
        reactionCounter++;

        movement = std::make_pair(food->getPosition().first - getPosition().first,
                                  food->getPosition().second - getPosition().second);
    }
}

// In Organism::interact
void Organism::interact(std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {
    printf("Organism %s is interacting with the other objects\n",
           boost::uuids::to_string(getId()).c_str());
    for (auto& object : interactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(object)) {
            if (!food->canBeEaten()) continue;

            std::cout << "Organism " << this->getId() << " is eating food. Life span: " << lifeSpan
                      << " + " << food->getEnergy() << std::endl;
            lifeSpan += food->getEnergy();
            food->eaten();
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
            printf(
                "Organism %s is interacting with organism %s, it will be %s after interaction. \n",
                boost::uuids::to_string(getId()).c_str(),
                boost::uuids::to_string(organism->getId()).c_str(),
                getSize() * 1.5 > organism->getSize() ? "killed" : "alive");

            if (getSize() * 1.5 > organism->getSize()) {
                lifeSpan += 300;
                organism->killed();
            }
        }
    }
}

std::shared_ptr<Organism> Organism::reproduce() {
    // copy the gene than mutate and create a new organism
    Genes newGenes = genes;
    newGenes.mutate();
    return std::make_shared<Organism>(newGenes, lifeConsumptionCalculator);
}

void Organism::killed() { lifeSpan = 0; }

void Organism::postIteration() {
    printf("Organism %s is at (%f, %f) %f %f (%f, %f, %f)\n",
           boost::uuids::to_string(getId()).c_str(), getPosition().first, getPosition().second,
           lifeSpan, getLifeConsumption(), getSpeed(), getSize(), getAwareness());

    lifeSpan -= getLifeConsumption();

    // printf("Organism %s life span: %f\n", boost::uuids::to_string(getId()).c_str(), lifeSpan);
    if (lifeSpan <= 0) {
        killed();
        return;
    }

    printf("Organism %s is at (%f, %f), movement: (%f, %f), reaction counter: %d\n",
           boost::uuids::to_string(getId()).c_str(), getPosition().first, getPosition().second,
           movement.first, movement.second, reactionCounter);

    makeMove();
}

// make move have to calculate the movement of the organism
// the movement length is determined by the organism's speed
void Organism::makeMove() {
    // Initialize local random engine and distributions
    static std::random_device rd;                    // Non-deterministic seed
    static std::mt19937 gen(rd());                   // Standard mersenne_twister_engine
    std::uniform_int_distribution<int> dis(0, 4);    // Distribution for movement decision
    std::uniform_int_distribution<int> move(-1, 1);  // Distribution for movement direction

    auto speed = getSpeed();

    if (reactionCounter == 0) {
        // Determine to keep the movement or generate a new one
        // 80% chance to keep the movement
        bool keepMovement = dis(gen) > 0;
        if (movement.first == 0 && movement.second == 0) {
            keepMovement = false;
        }

        if (!keepMovement) {
            movement = std::make_pair(move(gen) * speed, move(gen) * speed);
        }
    }

    // Adjust the movement length to not exceed the speed of the organism
    auto movementLength =
        std::sqrt(movement.first * movement.first + movement.second * movement.second);
    if (movementLength > speed) {
        double scalingFactor = speed / movementLength;
        movement.first *= scalingFactor;
        movement.second *= scalingFactor;
    }

    setPosition(getPosition().first + movement.first, getPosition().second + movement.second);

    reactionCounter = 0;  // Reset reaction counter after making a move
}
