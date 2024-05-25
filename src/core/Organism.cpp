#include <core/Food.hpp>
#include <core/Organism.hpp>

Organism::Organism(const Genes &genes, LifeConsumptionCalculator calculator)
    : BaseEnvironmentObject(0, 0),
      genes(genes),
      lifeConsumptionCalculator(calculator) {}

uint8_t Organism::getSpeed() const {
    return static_cast<uint8_t>(genes.getGene(0));
}

uint8_t Organism::getSize() const {
    return static_cast<uint8_t>(genes.getGene(1));
}

uint8_t Organism::getAwareness() const {
    return static_cast<uint8_t>(genes.getGene(2));
}

uint32_t Organism::getLifeConsumption() const {
    if (lifeConsumptionCalculator) {
        return lifeConsumptionCalculator(*this);
    }

    return getSpeed() * getSize() * getAwareness();
}

void Organism::react(std::shared_ptr<BaseEnvironmentObject> object) {
    if (auto other_organism = std::dynamic_pointer_cast<Organism>(object)) {
        if (getSize() < other_organism->getSize()) {
            // run away from the other organism
            culmulativelyMovement = std::make_pair(
                culmulativelyMovement.first + getPosition().first -
                    other_organism->getPosition().first,
                culmulativelyMovement.second + getPosition().second -
                    other_organism->getPosition().second);
            movementCounter += 1;
        } else if (getSize() > other_organism->getSize()) {
            // trace the other organism
            culmulativelyMovement = std::make_pair(
                culmulativelyMovement.first +
                    other_organism->getPosition().first - getPosition().first,
                culmulativelyMovement.second +
                    other_organism->getPosition().second -
                    getPosition().second);
            movementCounter += 1;
        }
    } else if (auto food = std::dynamic_pointer_cast<Food>(object)) {
        culmulativelyMovement =
            std::make_pair(culmulativelyMovement.first + getPosition().first -
                               food->getPosition().first,
                           culmulativelyMovement.second + getPosition().second -
                               food->getPosition().second);
    }
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

// make move have to calculate the movement of the organism
// the movement length is determined by the organism's speed
void Organism::makeMove() {
    if (movementCounter ==
        0) {  // if movementCounter is 0, then randomize movement
        auto speed = getSpeed();
        auto angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        culmulativelyMovement =
            std::make_pair(std::cos(angle) * speed, std::sin(angle) * speed);
        return;
    }

    auto movement =
        std::make_pair(culmulativelyMovement.first / movementCounter,
                       culmulativelyMovement.second / movementCounter);

    // make movement length equal to organism's speed
    auto speed = getSpeed();
    auto movementLength = std::sqrt(movement.first * movement.first +
                                    movement.second * movement.second);
    movement = std::make_pair(movement.first / movementLength * speed,
                              movement.second / movementLength * speed);

    culmulativelyMovement = std::make_pair(0, 0);
    movementCounter = 0;
}
