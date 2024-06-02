
import matplotlib.pyplot as plt
import numpy as np
from simevopy import Environment
from utils.common import (
    distribute_food_randomly,
    remove_all_foods,
    reproduce_organisms,
    setup_base_organism,
)
from utils.visualize import visualize_objects

def distribute_food_like_oasis(env, food_count):
    distribute_food_randomly(env, int(food_count / 4), (1600, 1600), (1800, 1800))
    distribute_food_randomly(env, int(food_count / 4), (0, 0), (2000, 2000))
    
    distribute_food_randomly(env, int(food_count / 2), (100, 100), (500, 500))
    distribute_food_randomly(env, int(food_count / 6), (1000, 1000), (1500, 1500))


env = Environment(2000, 2000)
setup_base_organism(env, 50)

# 初始化用於存儲每代數據的列表
organism_counts = []
average_sizes = []
average_speeds = []
average_awareness = []

for i in range(6000):
    print("=========================================")
    print(f"Gen {i} th")

    distribute_food_like_oasis(env, 100)

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

    print("After iteration, organism count: ", organism_count)

    if i % 100 == 0:
        visualize_objects(env, f"Organisms and Food Distribution (Generation {i})")
        input("Press Enter to continue...")

    reproduce_organisms(env)
    remove_all_foods(env)

# 繪製每代的結果圖表
plt.figure(figsize=(10, 8))
plt.subplot(4, 1, 1)
plt.plot(organism_counts, label="Organism Count")
plt.legend()

plt.subplot(4, 1, 2)
plt.plot(average_sizes, label="Average Size")
plt.legend()

plt.subplot(4, 1, 3)
plt.plot(average_speeds, label="Average Speed")
plt.legend()

plt.subplot(4, 1, 4)
plt.plot(average_awareness, label="Awareness Radius")
plt.legend()

plt.xlabel("Generation")
plt.show()
