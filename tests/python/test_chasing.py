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

    # Simulate without error
    try:
        for _ in range(100):
            env.simulate_iteration(1)
    except Exception as e:
        pytest.fail(f"Simulation failed with an exception: {e}")

    all_orgs = env.get_all_organisms()

    # Both organisms should still be alive (lifespan callback returns 0 consumption)
    assert len(all_orgs) == 2, "There should be exactly two organisms in the environment"
    assert all(org.is_alive() for org in all_orgs), "All organisms should still be alive"
