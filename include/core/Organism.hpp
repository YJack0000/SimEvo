#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "EnvironmentObject.hpp"
#include "Genes.hpp"

class Organism : public EnvironmentObject {
public:
    using LifeConsumptionCalculator = std::function<uint32_t(const Organism &)>;

    Organism();
    Organism(const Genes &genes);
    Organism(const Genes &genes, LifeConsumptionCalculator lifeConsumptionCalculator);

    // attributes
    float getSpeed() const;
    float getSize() const;
    float getAwareness() const;
    float getLifeConsumption() const;
    float getLifeSpan() const;
    float getReactionRadius() const;

    // status
    void killed();
    bool isAlive() const;
    bool canReproduce() const;

    ~Organism() {};

    // actions
    void react(const std::vector<std::shared_ptr<EnvironmentObject>> &reactableObjects);
    void interact(const std::vector<std::shared_ptr<EnvironmentObject>> &interactableObjects);
    std::shared_ptr<Organism> reproduce();

    void postIteration() override;

private:
    Genes genes;
    LifeConsumptionCalculator lifeConsumptionCalculator;
    float lifeSpan;

    double calculateDistance(const std::shared_ptr<EnvironmentObject> &object) const;

    std::pair<float, float> movement;
    int reactionCounter = 0;
    void makeMove();
};

#endif
