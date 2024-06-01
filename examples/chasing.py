from simevopy import Environment, Genes, Organism
from utils.visualize import visualize_objects

def custom_lifespan_callback(organism):
    return 0

env = Environment(1000, 1000)
dna = chr(25) * 1 + chr(30) * 1 + chr(100) * 2
dna2 = chr(25) * 1 + chr(80) + chr(120) * 2

env.add_organism(Organism(Genes(dna), custom_lifespan_callback), 100, 100)
env.add_organism(Organism(Genes(dna2), custom_lifespan_callback), 80, 80)
visualize_objects(env)
env.simulate_iteration(100, on_each_iteration=visualize_objects)

input("Press Enter to continue...")
