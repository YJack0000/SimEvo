from simevopy import Environment
from utils.common import (
    distribute_food_randomly,
    remove_foods,
    reproduce_organisms,
    setup_base_organism,
)
from utils.visualize import visualize_objects


def distribute_food_like_oasis(env, food_count):
    distribute_food_randomly(env, food_count / 2, (800, 800), (1200, 1200))
    distribute_food_randomly(env, food_count / 2, (600, 600), (1500, 1500))


env = Environment(2000, 2000)

setup_base_organism(env, 20)

for i in range(100):
    print("=========================================")
    print(f"Gen {i} th")

    distribute_food_like_oasis(env, 100)

    visualize_objects(env, f"Organisms and Food Distribution (Generation {i})")
    env.simulate_iteration(
        100,
        on_each_iteration=lambda env: visualize_objects(
            env, f"Organisms and Food Distribution (Generation {i})"
        ),
    )
    print("After iteration, organism count: ", len(env.get_all_organisms()))

    if len(env.get_all_foods()) < 0:
        print("No food left. Stopping the simulation.")
        break

    reproduce_organisms(env)
    remove_foods(env)

input("Press Enter to continue...")
