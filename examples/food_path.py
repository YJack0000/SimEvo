import math

from simevopy import Environment, Food, Genes, Organism
from utils.visualize import visualize_objects

def distribute_food_in_circle(env, food_count=30):
    for i in range(food_count):
        angle = i * 2 * math.pi / food_count
        x = 500 + math.cos(angle) * 300
        y = 500 + math.sin(angle) * 300
        env.add_food(Food(), x, y)


env = Environment(1000, 1000)
dna = chr(40) * 2 + chr(120) * 2

env.add_organism(Organism(Genes(dna)), 275, 275)
distribute_food_in_circle(env, food_count=80)
visualize_objects(env)
env.simulate_iteration(300, on_each_iteration=visualize_objects)

input("Press Enter to continue...")
