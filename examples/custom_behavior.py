"""
Example: Custom Python Behavior Strategies

Demonstrates how to define organism behavior entirely in Python
without modifying the C++ engine. Two species coexist:
- Herbivores: only eat food, flee from all organisms
- Predators: ignore food, chase smaller organisms
"""

import random

from simevopy import Environment, Food, Genes, Organism


# --- Custom Reaction Strategies ---

def herbivore_reaction(organism, nearby_objects):
    """Herbivores prioritize food, flee from any organism."""
    my_pos = organism.get_position()
    nearest_food = None
    nearest_food_dist = float("inf")
    nearest_org = None
    nearest_org_dist = float("inf")

    for obj in nearby_objects:
        dx = my_pos[0] - obj.get_position()[0]
        dy = my_pos[1] - obj.get_position()[1]
        dist = (dx * dx + dy * dy) ** 0.5

        if hasattr(obj, "can_be_eaten") and obj.can_be_eaten():
            if dist < nearest_food_dist:
                nearest_food = obj
                nearest_food_dist = dist
        elif hasattr(obj, "is_alive") and obj.is_alive():
            if dist < nearest_org_dist:
                nearest_org = obj
                nearest_org_dist = dist

    # Flee from organisms if one is very close
    if nearest_org and nearest_org_dist < 20:
        org_pos = nearest_org.get_position()
        return (my_pos[0] - org_pos[0], my_pos[1] - org_pos[1])

    # Otherwise move toward food
    if nearest_food:
        food_pos = nearest_food.get_position()
        return (food_pos[0] - my_pos[0], food_pos[1] - my_pos[1])

    return (0.0, 0.0)


def predator_reaction(organism, nearby_objects):
    """Predators chase smaller organisms, ignore food."""
    my_pos = organism.get_position()
    my_size = organism.get_size()

    nearest_prey = None
    nearest_prey_dist = float("inf")

    for obj in nearby_objects:
        if not hasattr(obj, "is_alive") or not obj.is_alive():
            continue
        if not hasattr(obj, "get_size"):
            continue
        if obj.get_size() >= my_size:
            continue

        dx = my_pos[0] - obj.get_position()[0]
        dy = my_pos[1] - obj.get_position()[1]
        dist = (dx * dx + dy * dy) ** 0.5

        if dist < nearest_prey_dist:
            nearest_prey = obj
            nearest_prey_dist = dist

    if nearest_prey:
        prey_pos = nearest_prey.get_position()
        return (prey_pos[0] - my_pos[0], prey_pos[1] - my_pos[1])

    return (0.0, 0.0)


# --- Custom Interaction Strategies ---

def herbivore_interaction(organism, nearby_objects):
    """Herbivores only eat food, never attack other organisms."""
    for obj in nearby_objects:
        if hasattr(obj, "can_be_eaten") and obj.can_be_eaten():
            organism.add_life_span(obj.get_energy())
            obj.eaten()


def predator_interaction(organism, nearby_objects):
    """Predators eat smaller organisms, ignore food."""
    for obj in nearby_objects:
        if not hasattr(obj, "is_alive") or not obj.is_alive():
            continue
        if not hasattr(obj, "get_size"):
            continue
        if organism.get_size() > 1.2 * obj.get_size():
            organism.add_life_span(obj.get_life_span())
            obj.killed()


def no_life_cost(organism):
    """Zero life consumption for demo purposes."""
    return 0


def main():
    env = Environment(500, 500)

    # Create herbivores (small, fast, high awareness)
    for _ in range(15):
        dna = chr(60) + chr(15) + chr(80) + chr(0)  # fast, small, aware
        org = Organism(Genes(dna), no_life_cost)
        org.set_reaction_strategy(herbivore_reaction)
        org.set_interaction_strategy(herbivore_interaction)
        env.add_organism(org, random.uniform(10, 490), random.uniform(10, 490))

    # Create predators (slower, bigger, less awareness)
    for _ in range(5):
        dna = chr(30) + chr(80) + chr(60) + chr(0)  # slow, big, moderate awareness
        org = Organism(Genes(dna), no_life_cost)
        org.set_reaction_strategy(predator_reaction)
        org.set_interaction_strategy(predator_interaction)
        env.add_organism(org, random.uniform(10, 490), random.uniform(10, 490))

    # Add food
    for _ in range(30):
        env.add_food(Food(), random.uniform(10, 490), random.uniform(10, 490))

    # Run simulation
    env.simulate_iteration(200)

    # Report results
    alive = env.get_all_organisms()
    print(f"\nAfter 200 iterations:")
    print(f"  Surviving organisms: {len(alive)}")
    print(f"  Remaining food: {len(env.get_all_foods())}")
    print(f"  Food consumed: {env.get_food_consumption_in_iteration()}")
    print(f"  Dead organisms: {len(env.get_dead_organisms())}")


if __name__ == "__main__":
    main()
