import math
import pytest
from simevopy import Environment, Food, Genes, Organism

def distribute_food_in_circle(env, food_count=30):
    for i in range(food_count):
        angle = i * 2 * math.pi / food_count
        x = 500 + math.cos(angle) * 300
        y = 500 + math.sin(angle) * 300
        env.add_food(Food(), x, y)

@pytest.fixture
def setup_environment():
    env = Environment(1000, 1000)
    dna = chr(40) * 2 + chr(120) * 2
    org = Organism(Genes(dna))
    env.add_organism(org, 275, 275)
    distribute_food_in_circle(env, food_count=80)
    return env

def test_food_distribution(setup_environment):
    env = setup_environment
    assert len(env.get_all_foods()) == 80, "There should be exactly 80 food particles in the environment"

def test_environment_simulation(setup_environment):
    env = setup_environment
    
    # Initial organism position check
    initial_position = env.get_all_organisms()[0].get_position()
    assert initial_position == (275, 275), "Organism should start at position (275, 275)"

    # Simulate without error
    try:
        env.simulate_iteration(300)
    except Exception as e:
        pytest.fail(f"Simulation failed with an exception: {e}")
    
    # Ensure the organism has moved
    remain_foods = env.get_all_foods()
    assert len(remain_foods) == 0, "All food particles should be consumed"
