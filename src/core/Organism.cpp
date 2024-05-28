#include <boost/uuid/uuid_io.hpp>
#include <core/Food.hpp>
#include <core/Organism.hpp>
#include <iostream>
#include <random>

Organism::Organism(const Genes &genes)
    : BaseEnvironmentObject(0, 0), genes(genes) {}

Organism::Organism(const Genes &genes, LifeConsumptionCalculator calculator)
    : BaseEnvironmentObject(0, 0),
      genes(genes),
      lifeConsumptionCalculator(calculator) {}

float Organism::getSpeed() const {
    return (float)genes.getGene(0) / 10;  // 0~12.8
}

float Organism::getSize() const {
    return (float)genes.getGene(1) / 10;  // 0~12.8
}

float Organism::getAwareness() const {
    return (float)genes.getGene(2) / 10;  // 0~12.8
}

float Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

    return getSpeed() * getSize() * getAwareness() / 10;
}

float Organism::getReactionRadius() const { return getAwareness(); }

bool Organism::isAlive() const { return lifeSpan > 0; }

void Organism::react(std::shared_ptr<BaseEnvironmentObject> object) {
    if (auto other_organism = std::dynamic_pointer_cast<Organism>(object)) {
        if (reactionCounter != 0) {  // already decided to move
            return;
        }
        reactionCounter++;
        // react only once to one organism, however the food reaction can be
        // overriden by organism reaction

        if (getSize() < other_organism->getSize()) {
            // run away from the other organism
            movement = std::make_pair(
                getPosition().first - other_organism->getPosition().first,
                getPosition().second - other_organism->getPosition().second);
        } else if (getSize() > other_organism->getSize()) {
            // trace the other organism
            movement = std::make_pair(
                other_organism->getPosition().first - getPosition().first,
                other_organism->getPosition().second - getPosition().second);
        }
    } else if (auto food =
                   std::dynamic_pointer_cast<Food>(object)) {  // close to food
        if (!food->canBeEaten()) return;
        reactionCounter++;

        movement =
            std::make_pair(food->getPosition().first - getPosition().first,
                           food->getPosition().second - getPosition().second);
    }
}

void Organism::interact(std::shared_ptr<BaseEnvironmentObject> object) {
    if (auto food = std::dynamic_pointer_cast<Food>(object)) {
        if (!food->canBeEaten()) return;

        std::cout << "Organism " << this->getId()
                  << " is eating food. Life span: " << lifeSpan << "+ 300!"
                  << std::endl;
        lifeSpan += 300;
        food->eaten();
    }

    if (auto organism = std::dynamic_pointer_cast<Organism>(object)) {
        std::cout << "Organism (" << this->getId() << ", " << getSize()
                  << ")  interact with (" << object->getId() << ", "
                  << organism->getSize() << ")" << std::endl;
        if (getSize() > organism->getSize()) {
            lifeSpan += 300;
            organism->killed();
        }
    }
}

void Organism::killed() { lifeSpan = 0; }

void Organism::postIteration() {
    std::cout << "Organism " << this->getId()
              << " is post iteration. Life span: " << lifeSpan << " - "
              << getLifeConsumption() << std::endl;
    lifeSpan -= getLifeConsumption();
    if (lifeSpan <= 0) {
        killed();
        return;
    }

    makeMove();
}

// make move have to calculate the movement of the organism
// the movement length is determined by the organism's speed
void Organism::makeMove() {
    // Initialize local random engine and distributions
    static std::random_device rd;  // Non-deterministic seed
    static std::mt19937 gen(rd()); // Standard mersenne_twister_engine
    std::uniform_int_distribution<int> dis(0, 1);  // Distribution for movement decision
    std::uniform_int_distribution<int> move(-1, 1); // Distribution for movement direction

    auto speed = getSpeed();

    if (reactionCounter == 0) {
        // Determine to keep the movement or generate a new one
        bool keepMovement = dis(gen);
        if (movement.first == 0 && movement.second == 0) {
            keepMovement = false;
        }

        if (!keepMovement) {
            movement = std::make_pair(move(gen) * speed, move(gen) * speed);
        }
    }

    // Adjust the movement length to not exceed the speed of the organism
    auto movementLength = std::sqrt(movement.first * movement.first + movement.second * movement.second);
    if (movementLength > speed) {
        double scalingFactor = speed / movementLength;
        movement.first *= scalingFactor;
        movement.second *= scalingFactor;
    }

    setPosition(getPosition().first + movement.first, getPosition().second + movement.second);

    reactionCounter = 0;  // Reset reaction counter after making a move
}
