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
 * Organism behaviour is driven by two pluggable strategy callbacks:
 *  - **ReactionStrategy** -- decides movement direction based on nearby objects.
 *  - **InteractionStrategy** -- performs close-range actions (eating, fighting).
 *
 * When no custom strategy is set, built-in defaults are used. Custom strategies
 * are inherited by offspring produced via reproduce(), enabling Python-side
 * behaviour injection that persists across generations.
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

    /**
     * @brief Strategy that decides how an organism reacts to nearby objects.
     *
     * Given a reference to the organism and a list of nearby objects (within
     * the reaction radius), the strategy returns a (dx, dy) movement direction.
     * Returning {0, 0} signals "no reaction" and the organism keeps wandering.
     */
    using ReactionStrategy = std::function<std::pair<float, float>(
        Organism &, const std::vector<std::shared_ptr<EnvironmentObject>> &)>;

    /**
     * @brief Strategy that defines close-range interactions with nearby objects.
     *
     * Given a reference to the organism and objects within its size radius,
     * the strategy mutates state directly (e.g. consuming food, killing smaller
     * organisms). The default eats food and preys on organisms less than 2/3 its size.
     */
    using InteractionStrategy = std::function<void(
        Organism &, const std::vector<std::shared_ptr<EnvironmentObject>> &)>;

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

    /**
     * @brief Add (or subtract) life-span points.
     *
     * Exposed publicly so that custom InteractionStrategy callbacks (including
     * those written in Python) can reward or penalise organisms.
     *
     * @param amount Points to add; negative values reduce life-span.
     */
    void addLifeSpan(float amount);

    ~Organism() {};

    // ── Behaviour injection ─────────────────────────────────────────────

    /**
     * @brief Replace the reaction strategy with a custom implementation.
     *
     * The strategy is propagated to offspring during reproduce(). Pass nullptr
     * or an empty std::function to revert to the built-in default.
     *
     * @param strategy Callable matching the ReactionStrategy signature.
     */
    void setReactionStrategy(ReactionStrategy strategy);

    /**
     * @brief Replace the interaction strategy with a custom implementation.
     *
     * The strategy is propagated to offspring during reproduce(). Pass nullptr
     * or an empty std::function to revert to the built-in default.
     *
     * @param strategy Callable matching the InteractionStrategy signature.
     */
    void setInteractionStrategy(InteractionStrategy strategy);

    /**
     * @brief Check whether this organism has any custom (non-default) strategy set.
     * @return true if either reactionStrategy or interactionStrategy is set.
     *
     * Used by Environment to decide whether multi-threaded execution is safe.
     * Custom strategies may involve Python callbacks that require the GIL.
     */
    bool hasCustomStrategy() const;

    // ── Actions ─────────────────────────────────────────────────────────

    /**
     * @brief Decide movement direction based on nearby objects within reaction radius.
     * @param reactableObjects Objects detected within the organism's reaction radius.
     *
     * Delegates to the custom ReactionStrategy if one has been set, otherwise
     * falls back to defaultReaction(). Only triggers once per iteration
     * (guarded by reactionCounter).
     */
    void react(const std::vector<std::shared_ptr<EnvironmentObject>> &reactableObjects);

    /**
     * @brief Interact with objects within the organism's body size range.
     * @param interactableObjects Objects overlapping the organism's size radius.
     *
     * Delegates to the custom InteractionStrategy if set, otherwise uses
     * defaultInteraction(). Must run single-threaded because it mutates
     * shared state (food eaten flags, other organisms' lifespans).
     */
    void interact(const std::vector<std::shared_ptr<EnvironmentObject>> &interactableObjects);

    /**
     * @brief Create a mutated offspring organism.
     * @return A new organism with mutated genes, inheriting the life consumption
     *         calculator and any custom behavior strategies.
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
    ReactionStrategy reactionStrategy;                     ///< Optional custom reaction behaviour
    InteractionStrategy interactionStrategy;               ///< Optional custom interaction behaviour
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

    // Default built-in strategies
    static std::pair<float, float> defaultReaction(
        Organism &self, const std::vector<std::shared_ptr<EnvironmentObject>> &objects);
    static void defaultInteraction(
        Organism &self, const std::vector<std::shared_ptr<EnvironmentObject>> &objects);
};

#endif
