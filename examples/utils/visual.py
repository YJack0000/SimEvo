import numpy as np
from simevopy import Organism, Food
from matplotlib import pyplot as plt

def visualize_objects(env):
    objects = env.get_all_objects()
    organisms_positions = []
    food_positions = []
    awareness_radii = []

    for obj in objects:
        if isinstance(obj, Organism):
            organisms_positions.append(obj.get_position())
            awareness_radii.append(obj.get_reaction_radius())
        elif isinstance(obj, Food):
            food_positions.append(obj.get_position())

    organisms_positions = np.array(organisms_positions)
    food_positions = np.array(food_positions)
    awareness_radii = np.array(awareness_radii)

    # print(organisms_positions)
    # print(food_positions)
    # print(awareness_radii)

    plt.clf()
    # 檢查 organisms_positions 是否為空，不為空時繪圖
    if organisms_positions.size > 0:
        plt.scatter(organisms_positions[:, 0], organisms_positions[:, 1], s=1, color='blue', label='Organisms')
        for i in range(len(organisms_positions)):
            circle = plt.Circle((organisms_positions[i, 0], organisms_positions[i, 1]), awareness_radii[i], color='red', fill=False, alpha=0.5)
            plt.gca().add_artist(circle)

    # 檢查 food_positions 是否為空，不為空時繪圖
    if food_positions.size > 0:
        plt.scatter(food_positions[:, 0], food_positions[:, 1], s=1, color='green', label='Food')

    plt.xlim(0, env.get_width())
    plt.ylim(0, env.get_height())
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.legend()
    plt.title('Organisms and Food Distribution')
    plt.draw()
    plt.pause(0.1)
