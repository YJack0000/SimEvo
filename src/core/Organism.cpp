#include <boost/uuid/uuid_io.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <cstdio>
#include <iostream>
#include <random>

Organism::Organism() : EnvironmentObject(0, 0), genes("\x14\x14\x14\x14"), lifeSpan(500) {}

Organism::Organism(const Genes& genes) : EnvironmentObject(0, 0), genes(genes), lifeSpan(500) {}

Organism::Organism(const Genes& genes, LifeConsumptionCalculator calculator)
    : EnvironmentObject(0, 0), genes(genes), lifeConsumptionCalculator(calculator), lifeSpan(500) {}

float Organism::getSpeed() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(0))) / 4.0f;  // 256 / 4 = 64
}

float Organism::getSize() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(1))) / 4.0f;  // 256 / 4 = 64
}

float Organism::getAwareness() const {
    return static_cast<float>(static_cast<unsigned char>(genes.getDNA(2))) / 4.0f;  // 256 / 4 = 64
}

float Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

    // printf("Organism %s is getting speed %f, size %f, awareness %f\n",
    //  boost::uuids::to_string(getId()).c_str(), getSpeed(), getSize(), getAwareness());
    float consumption =
        (getSpeed() / 10 * getSpeed() / 10 + getSize() / 10 * getSize() / 10 * getSize() / 15 +
        getAwareness() / 10) * 1.3;
    // printf("Organism %s is getting life consumption %f\n",
    //        boost::uuids::to_string(getId()).c_str(), consumption);
    return consumption;
}

float Organism::getLifeSpan() const { return lifeSpan; }

float Organism::getReactionRadius() const { return getSize() + getAwareness(); }

bool Organism::isAlive() const { return lifeSpan > 0; }

bool Organism::canReproduce() const { return lifeSpan > 1000; } 

// bool Organism::isFull() const {
//     printf("Organism thinking whether it is full\n");
//     return lifeSpan > getSize() * 50 * 2;
// }  // *50 nom to lifespan and *2 to at least reproduce

double Organism::calculateDistance(std::shared_ptr<EnvironmentObject> object) {
    auto dx = getPosition().first - object->getPosition().first;
    auto dy = getPosition().second - object->getPosition().second;
    return std::sqrt(dx * dx + dy * dy);
}

void Organism::react(std::vector<std::shared_ptr<EnvironmentObject>>& reactableObjects) {
    if (reactableObjects.empty()) {
        // printf("Organism %s is not reacting to any objects\n",
        //  boost::uuids::to_string(getId()).c_str());
        return;
    }
    // printf("Organism %s is reacting to the other objects\n",
    //  boost::uuids::to_string(getId()).c_str());

    std::shared_ptr<EnvironmentObject> nearestObject = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    // printf("Organism %s is calculating the nearest object\n",
    // boost::uuids::to_string(getId()).c_str());

    // Find the nearest valid object
    for (const auto& obj : reactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(obj)) {
            if (!food->canBeEaten()) {
                // printf("Organism %s is ignoring the food\n", boost::uuids::to_string(getId()).c_str());
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
        // printf("Organism %s found no valid objects to react to\n",
        // boost::uuids::to_string(getId()).c_str());
        return;
    }

    if (auto otherOrganism = std::dynamic_pointer_cast<Organism>(nearestObject)) {
        if (reactionCounter != 0) {  // already decided to move
            return;
        }

        if (getSize() * 1.5 < otherOrganism->getSize()) {
            // printf("Organism %s is running away from organism %s\n",
            //        boost::uuids::to_string(getId()).c_str(),
            //        boost::uuids::to_string(otherOrganism->getId()).c_str());
            // run away from the other organism
            movement = std::make_pair(getPosition().first - otherOrganism->getPosition().first,
                                      getPosition().second - otherOrganism->getPosition().second);
            reactionCounter++;
        } else if (getSize() > 1.5 * otherOrganism->getSize()) {
            // printf("Organism %s is chasing organism %s\n",
            // boost::uuids::to_string(getId()).c_str(),
            //        boost::uuids::to_string(otherOrganism->getId()).c_str());
            // trace the other organism
            movement = std::make_pair(otherOrganism->getPosition().first - getPosition().first,
                                      otherOrganism->getPosition().second - getPosition().second);
            reactionCounter++;
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(nearestObject)) {
        if (!food->canBeEaten()) {
            return;
        }

        // if (isFull()) {
        // if (lifeSpan > 1200) {
            // printf("Organism of size %f is full\n", getSize());
            // This constraint is added to prevent the organism from eating food when it is full
        //     return;
        // }

        // printf("Organism %s is moving to the food \n", boost::uuids::to_string(getId()).c_str());
        reactionCounter++;

        movement = std::make_pair(food->getPosition().first - getPosition().first,
                                  food->getPosition().second - getPosition().second);
    } else {
        throw std::runtime_error("Unknown object type detected");
    }
}

// In Organism::interact
void Organism::interact(std::vector<std::shared_ptr<EnvironmentObject>>& interactableObjects) {
    // printf("Organism %s is interacting with the other objects\n",
    //  boost::uuids::to_string(getId()).c_str());
    for (auto& object : interactableObjects) {
        if (auto food = std::dynamic_pointer_cast<Food>(object)) {
            if (!food->canBeEaten()) continue;

            // std::cout << "Organism " << this->getId() << " is eating food. Life span: " <<
            // lifeSpan
            //           << " + " << food->getEnergy() << std::endl;
            lifeSpan += food->getEnergy();
            food->eaten();
        }

        if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
            // printf(
            //  "Organism %s is interacting with organism %s, it will be %s after interaction. \n",
            //  boost::uuids::to_string(getId()).c_str(),
            //  boost::uuids::to_string(organism->getId()).c_str(),
            //  getSize() > 4 * organism->getSize() ? "killed" : "alive");
            // printf("Organism %s is %f, %f, %f, organism %s is %f, %f, %f\n",
            //  boost::uuids::to_string(getId()).c_str(), getSpeed(), getSize(), getAwareness(),
            //  boost::uuids::to_string(organism->getId()).c_str(), organism->getSpeed(),
            //  organism->getSize(), organism->getAwareness());

            if (getSize() > 1.5 * organism->getSize()) {
                lifeSpan += organism->getLifeSpan();
                organism->killed();
            }
        }
    }
}

std::shared_ptr<Organism> Organism::reproduce() {
    // copy the gene than mutate and create a new organism
    // printf("Organism %s is reproducing, it attr is (%f, %f, %f) \n",
    //        boost::uuids::to_string(getId()).c_str(), getSpeed(), getSize(), getAwareness());
    Genes newGenes = genes;
    newGenes.mutate();
    auto newOrganism = std::make_shared<Organism>(newGenes, lifeConsumptionCalculator);
    // printf("New organism %s is created, it attr is (%f, %f, %f) \n",
    //  boost::uuids::to_string(newOrganism->getId()).c_str(), newOrganism->getSpeed(),
    //  newOrganism->getSize(), newOrganism->getAwareness());
    newOrganism->setPosition(getPosition().first + 2, getPosition().second + 2);
    lifeSpan /= 2;
    return newOrganism;
}

void Organism::killed() { lifeSpan = 0; }

void Organism::postIteration() {
    // printf("Organism %s is at (%f, %f) %f %f (%f, %f, %f)\n",
    //  boost::uuids::to_string(getId()).c_str(), getPosition().first, getPosition().second,
    //  lifeSpan, getLifeConsumption(), getSpeed(), getSize(), getAwareness());

    lifeSpan -= getLifeConsumption();

    // printf("Organism %s life span: %f\n", boost::uuids::to_string(getId()).c_str(), lifeSpan);
    if (lifeSpan <= 0) {
        killed();
        return;
    }

    // printf("Organism %s is at (%f, %f), movement: (%f, %f), reaction counter: %d\n",
    //  boost::uuids::to_string(getId()).c_str(), getPosition().first, getPosition().second,
    //  movement.first, movement.second, reactionCounter);

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
