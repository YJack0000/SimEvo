from SimSel import Environment, Organism, Genes
import random

def setup_base_organism(env, count=10):
    for _ in range(count):
        x = random.randint(0, env.width - 1)
        y = random.randint(0, env.height - 1)
        env.add_organism(Organism(Genes("ABCD")), x, y)

def distribute_food_randomly(env, food_count=50):
    for _ in range(food_count):
        x = random.randint(0, env.width - 1)
        y = random.randint(0, env.height - 1)
        env.add_food(x, y)

env = Environment(500, 500, type="optimize")

setup_base_organism(env)

for i in range(100):
    print(f"Gen {i} th")
    distribute_food_randomly(env, 100)
    env.simulate_iteration(50)
