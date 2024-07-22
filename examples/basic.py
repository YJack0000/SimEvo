from simevopy import Environment
from utils.common import (
    distribute_food_randomly,
    remove_all_foods,
    reproduce_organisms,
    setup_base_organism,
)
from utils.visualize import visualize_objects

env = Environment(1000, 1000, type="optimized")
setup_base_organism(env, 50)

for i in range(100):
    print("=========================================")
    print(f"Gen {i} th")

    distribute_food_randomly(env, 50)

    # visualize_objects(env, f"Organisms and Food Distribution (Generation {i})")
    env.simulate_iteration(
        100,
        on_each_iteration=lambda env: visualize_objects(
            env, f"Organisms and Food Distribution (Generation {i})"
        ),
    )

    print("After iteration, organism count: ", len(env.get_all_organisms()))

    reproduce_organisms(env)
    remove_all_foods(env)

    # input("Press Enter to continue...")
