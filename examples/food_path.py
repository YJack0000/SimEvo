import math

from simevopy import Environment, Food, Genes, Organism
from utils.visualize import visualize_objects


class NeverEnoughFoodOrganism(Organism):
    def __init__(self, genes):
        super().__init__(genes)

    def is_full(self):
        print("never enough food")
        return False


def distribute_food_in_circle(env, food_count=30):
    for i in range(food_count):
        angle = i * 2 * math.pi / food_count
        x = 500 + math.cos(angle) * 300
        y = 500 + math.sin(angle) * 300
        env.add_food(Food(), x, y)


env = Environment(1000, 1000, type="optimized")
dna = chr(40) * 2 + chr(80) * 2

try_new_org = NeverEnoughFoodOrganism(Genes(dna))
print(try_new_org.is_full())

env.add_organism(NeverEnoughFoodOrganism(Genes(dna)), 275, 275)
distribute_food_in_circle(env, food_count=50)
visualize_objects(env)
env.simulate_iteration(300, on_each_iteration=visualize_objects)

input("Press Enter to continue...")
