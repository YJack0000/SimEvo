#include <core/Organism.hpp>

Organism::Organism(const Genes &genes, LifeConsumptionCalculator calculator)
    : genes(genes), lifeConsumptionCalculator(calculator) {}

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

bool Organism::isAlive() const { return life < getLifeConsumption(); }

bool Organism::canReproduce() const { return life >= getLifeConsumption() * 2; }

void Organism::extendLife(int amount) { life += amount; }

void Organism::interact(Organism &other) {
    if (!isAlive() || !other.isAlive()) // one of the organisms is dead
        return;
}

void Organism::interact(Food &food) {
    if (!isAlive()) // the organism is dead
        return;

    extendLife(10);
}
