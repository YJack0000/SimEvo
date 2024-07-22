import numpy as np
from simevopy import Environment
from utils.common import (
    distribute_food_randomly,
    remove_all_foods,
    reproduce_organisms,
    setup_base_organism,
)

def distribute_food_like_oasis(env, food_count):
    distribute_food_randomly(env, int(food_count / 4), (1600, 1600), (1800, 1800))
    distribute_food_randomly(env, int(food_count / 4), (0, 0), (2000, 2000))
    
    distribute_food_randomly(env, int(food_count / 2), (100, 100), (500, 500))
    distribute_food_randomly(env, int(food_count / 6), (1000, 1000), (1500, 1500))


env = Environment(4000, 4000, threads=4)
setup_base_organism(env, 200)

# 初始化用於存儲每代數據的列表
organism_counts = []
average_sizes = []
average_speeds = []
average_awareness = []

for i in range(10):
    print("=========================================")
    print(f"Gen {i} th")

    distribute_food_like_oasis(env, 400)

    env.simulate_iteration(
        100,
    )

    organisms = env.get_all_organisms()
    organism_count = len(organisms)
    organism_counts.append(organism_count)

    if organism_count > 0:
        sizes = np.array([org.get_size() for org in organisms])
        speeds = np.array([org.get_speed() for org in organisms])
        awareness = np.array([org.get_awareness() for org in organisms])

        average_sizes.append(np.mean(sizes))
        average_speeds.append(np.mean(speeds))
        average_awareness.append(np.mean(awareness))
    else:
        average_sizes.append(0)
        average_speeds.append(0)
        average_awareness.append(0)

    reproduce_organisms(env)
    remove_all_foods(env)
