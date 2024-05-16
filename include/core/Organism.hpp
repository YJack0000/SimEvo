#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <cstdint>
#include <functional>

#include "Genes.hpp"

class Organism {
public:
    using LifeConsumptionCalculator = std::function<uint32_t(const Organism &)>;

    Organism(const Genes &genes, LifeConsumptionCalculator calculator);

    uint8_t getSpeed() const;
    uint8_t getSize() const;
    uint8_t getAwareness() const;
    uint32_t getLifeConsumption() const;

    bool isAlive() const;
    bool canReproduce() const;
    void eat();

private:
    Genes genes;
    LifeConsumptionCalculator lifeConsumptionCalculator;
    uint8_t foodCount = 0;
};

#endif
