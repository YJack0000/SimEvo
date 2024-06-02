from simevopy import Environment
from utils.common import (
    distribute_food_randomly,
    remove_all_foods,
    reproduce_organisms,
    setup_base_organism,
)
from utils.visualize import visualize_objects


def distribute_food_like_oasis(env, food_count):
    distribute_food_randomly(env, int(food_count / 3), (800, 800), (1200, 1200))
    distribute_food_randomly(env, int(food_count / 3), (400, 400), (1700, 1700))
    distribute_food_randomly(env, int(food_count / 3), (0, 0), (2000, 2000))


env = Environment(2000, 2000)

setup_base_organism(env, 50)

for i in range(200):
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


    reproduce_organisms(env)
    remove_all_foods(env)

input("Press Enter to continue...")
