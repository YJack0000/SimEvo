#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <cstdint>
#include <functional>
#include <memory>

#include "Food.hpp"
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
    void extendLife(int amount);

    void interact(std::shared_ptr<Organism> &other);
    void interact(std::shared_ptr<Food> &food);

private:
    Genes genes;
    LifeConsumptionCalculator lifeConsumptionCalculator;
    uint8_t life = 0;
};

#endif
