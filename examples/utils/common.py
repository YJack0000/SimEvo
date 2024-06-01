import random 
from simevopy import Environment, Food, Genes, Organism

def setup_base_organism(env: Environment, count=20, start=(0, 0), end=(1000, 1000)):
    if end == (0, 0):
        end = (env.get_width(), env.get_height())

    if end[0] < start[0] or end[1] < start[1]:
        raise ValueError("End point should be greater than start point")

    for _ in range(count):
        x = random.randint(start[0], end[0] - 1)
        y = random.randint(start[1], end[1] - 1)

        dna = chr(40) * 4
        env.add_organism(Organism(Genes(dna)), x, y)

def distribute_food_randomly(env: Environment, food_count=50, start=(0, 0), end=(0, 0)):
    if end == (0, 0):
        end = (env.get_width(), env.get_height())

    if end[0] < start[0] or end[1] < start[1]:
        raise ValueError("End point should be greater than start point")

    for _ in range(food_count):
        x = random.randint(start[0], end[0] - 1)
        y = random.randint(start[1], end[1] - 1)

        env.add_food(Food(), x, y)

def reproduce_organisms(env: Environment):
    organisms = env.get_all_organisms()
    for org in organisms:
        if org.can_reproduce():
            new_org = org.reproduce()
            env.add_organism(new_org, org.get_position()[0], org.get_position()[1])

def remove_all_foods(env: Environment):
    remaining_food = env.get_all_foods()
    for f in remaining_food:
        env.remove_food(f)
