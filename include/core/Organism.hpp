#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "EnvironmentObject.hpp"
#include "Genes.hpp"

/**
 * @brief A living entity in the simulation that can move, eat, fight, and reproduce.
 *
 * Organisms have gene-derived attributes (speed, size, awareness) that determine
 * their behavior and survival. Each iteration, organisms react to nearby objects
 * (deciding movement direction) and interact with overlapping objects (eating food,
 * killing smaller organisms). Reproduction creates a mutated offspring and halves
 * the parent's lifespan.
 */
class Organism : public EnvironmentObject {
public:
    /**
     * @brief Callable that computes per-iteration life consumption from organism attributes.
     *
     * When set, overrides the default quadratic cost formula. Allows Python or
     * C++ callers to define custom energy expenditure rules.
     */
    using LifeConsumptionCalculator = std::function<uint32_t(const Organism &)>;

    /** @brief Construct a default organism with preset genes and 500 lifespan. */
    Organism();

    /**
     * @brief Construct an organism with the given genes.
     * @param genes The genetic data determining speed, size, and awareness.
     */
    Organism(const Genes &genes);

    /**
     * @brief Construct an organism with genes and a custom life consumption formula.
     * @param genes The genetic data determining speed, size, and awareness.
     * @param lifeConsumptionCalculator Custom function to compute per-tick life drain.
     */
    Organism(const Genes &genes, LifeConsumptionCalculator lifeConsumptionCalculator);

    /** @brief Get movement speed derived from gene index 0 (DNA byte / 4.0). */
    float getSpeed() const;

    /** @brief Get body size derived from gene index 1 (DNA byte / 4.0). */
    float getSize() const;

    /** @brief Get awareness radius derived from gene index 2 (DNA byte / 4.0). */
    float getAwareness() const;

    /**
     * @brief Get per-iteration life consumption.
     * @return Life consumed per tick; uses custom calculator if set, otherwise
     *         a default quadratic formula based on speed, size, and awareness.
     */
    float getLifeConsumption() const;

    /** @brief Get the current remaining lifespan. */
    float getLifeSpan() const;

    /**
     * @brief Get the radius within which this organism can react to objects.
     * @return Sum of size and awareness attributes.
     */
    float getReactionRadius() const;

    /** @brief Mark this organism as dead (lifespan set to 0). */
    void killed();

    /**
     * @brief Check whether this organism is still alive.
     * @return true if lifespan is greater than 0.
     */
    bool isAlive() const;

    /**
     * @brief Check whether this organism has enough lifespan to reproduce.
     * @return true if lifespan exceeds the reproduction threshold (1000).
     */
    bool canReproduce() const;

    ~Organism() {};

    /**
     * @brief Decide movement direction based on nearby objects within reaction radius.
     * @param reactableObjects Objects detected within the organism's reaction radius.
     *
     * Finds the nearest valid object and sets movement direction accordingly:
     * flee from larger organisms, chase smaller organisms, move toward food.
     * Only triggers once per iteration (guarded by reactionCounter).
     * Safe to call in parallel -- only writes to this organism's own fields.
     */
    void react(const std::vector<std::shared_ptr<EnvironmentObject>> &reactableObjects);

    /**
     * @brief Interact with objects within the organism's body size range.
     * @param interactableObjects Objects overlapping the organism's size radius.
     *
     * Eats available food (gaining its energy) and kills smaller organisms
     * (absorbing their remaining lifespan). Must run single-threaded because
     * it mutates shared state (food eaten flags, other organisms' lifespans).
     */
    void interact(const std::vector<std::shared_ptr<EnvironmentObject>> &interactableObjects);

    /**
     * @brief Create a mutated offspring organism.
     * @return A new organism with mutated genes, inheriting the life consumption calculator.
     *
     * The parent's lifespan is halved. The child is placed at a small offset
     * from the parent's position.
     */
    std::shared_ptr<Organism> reproduce();

    /**
     * @brief End-of-iteration hook: deduct life consumption, kill if depleted, then move.
     */
    void postIteration() override;

private:
    Genes genes;                                           ///< Genetic data driving attributes
    LifeConsumptionCalculator lifeConsumptionCalculator;   ///< Optional custom life drain formula
    float lifeSpan;                                        ///< Remaining life points

    /**
     * @brief Compute Euclidean distance to another environment object.
     * @param object The target object.
     * @return Distance in environment coordinate units.
     */
    double calculateDistance(const std::shared_ptr<EnvironmentObject> &object) const;

    Vec2 movement;              ///< Current movement direction vector
    int reactionCounter = 0;    ///< Guards against multiple reactions per tick

    /**
     * @brief Apply the current movement vector to update position.
     *
     * If no reaction occurred this tick, either keeps the previous movement
     * (80% chance) or picks a new random direction. Normalizes movement to
     * not exceed the organism's speed.
     */
    void makeMove();
};

#endif
