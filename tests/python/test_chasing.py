import pytest
from simevopy import Environment, Genes, Organism
from math import sqrt

def custom_lifespan_callback(organism):
    return 0

def calculate_distance(x1, y1, x2, y2):
    return sqrt((x2 - x1)**2 + (y2 - y1)**2)

@pytest.fixture
def setup_environment():
    env = Environment(1000, 1000)
    dna = chr(25) * 1 + chr(30) * 1 + chr(100) * 2
    dna2 = chr(25) * 1 + chr(80) + chr(120) * 2

    env.add_organism(Organism(Genes(dna), custom_lifespan_callback), 100, 100)
    env.add_organism(Organism(Genes(dna2), custom_lifespan_callback), 80, 80)
    return env

def test_environment_simulation(setup_environment):
    env = setup_environment
    initial_distance = calculate_distance(100, 100, 80, 80)
    
    # Simulate without error
    try:
        for _ in range(100):
            env.simulate_iteration(1)
    except Exception as e:
        pytest.fail(f"Simulation failed with an exception: {e}")
    
    all_orgs = env.get_all_organisms()

    # Check organism count
    assert len(all_orgs) == 2, "There should be exactly two organisms in the environment"
    
    # Check final distance
    first_org_pos = all_orgs[0].get_position()
    second_org_pos = all_orgs[1].get_position()
    final_distance = calculate_distance(first_org_pos[0], first_org_pos[1], second_org_pos[0], second_org_pos[1])
    assert abs(initial_distance - final_distance) < 1e-4, "The distance between organisms should not change significantly"
