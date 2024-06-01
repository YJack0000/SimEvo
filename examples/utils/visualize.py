import matplotlib.pyplot as plt
import numpy as np

def visualize_objects(env, title="Objects Distribution"):
    # Get all eatable foods
    foods = env.get_all_foods()
    food_positions = np.array([food.get_position() for food in foods if food.can_be_eaten()])
    # Get all alive organisms
    organisms = env.get_all_organisms()
    organisms_positions = np.array([org.get_position() for org in organisms if org.is_alive()])

    awareness_radii = np.array(
        [org.get_reaction_radius() for org in env.get_all_organisms()]
    )
    organisms_size = np.array([org.get_size() for org in env.get_all_organisms()])
    # life_span = np.array([org.get_life_span() for org in env.get_all_organisms()])

    plt.clf()
    # 檢查 organisms_positions 是否為空，不為空時繪圖
    if organisms_positions.size > 0:
        for i in range(len(organisms_positions)):
            circle = plt.Circle(
                (organisms_positions[i, 0], organisms_positions[i, 1]),
                organisms_size[i],
                color="blue",
                alpha=0.5,
            )
            plt.gca().add_artist(circle)

            circle = plt.Circle(
                (organisms_positions[i, 0], organisms_positions[i, 1]),
                awareness_radii[i],
                color="red",
                fill=False,
                alpha=0.5,
            )
            plt.gca().add_artist(circle)

    # 檢查 food_positions 是否為空，不為空時繪圖
    if food_positions.size > 0:
        plt.scatter(
            food_positions[:, 0], food_positions[:, 1], s=1, color="green", label="Food"
        )

    plt.xlim(0, env.get_width())
    plt.ylim(0, env.get_height())
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.legend()
    plt.title(title)
    plt.draw()
    plt.pause(0.005)
    # input("Press Enter to continue...")

