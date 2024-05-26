#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <cstdint>
#include <functional>
#include <memory>

#include "BaseEnvironmentObject.hpp"
#include "Genes.hpp"

class Organism : public BaseEnvironmentObject {
public:
    using LifeConsumptionCalculator = std::function<uint32_t(const Organism &)>;

    Organism(const Genes &genes,
             LifeConsumptionCalculator lifeConsumptionCalculator);

    uint8_t getSpeed() const;
    uint8_t getSize() const;
    uint8_t getAwareness() const;
    uint32_t getLifeConsumption() const;

    void killed();

    bool isAlive() const;
    bool canReproduce() const;

    // virtual
    ~Organism(){};
    int getReactionRadius() const;
    void interact(std::shared_ptr<BaseEnvironmentObject> object);
    void react(std::shared_ptr<BaseEnvironmentObject> object);
    void postIteration() override;

    // [TODO] change this - very bad implementation in order to make organism
    // move
    std::pair<float, float> getPosition() const override;
    void setPosition(float x, float y) override;

private:
    Genes genes;
    LifeConsumptionCalculator lifeConsumptionCalculator;
    int lifeSpan = 0;

    // [TODO] change this - very bad implementation in order to make organism
    // move
    std::pair<float, float> culmulativelyMovement;
    int movementCounter = 0;
    void makeMove();
};

#endif
