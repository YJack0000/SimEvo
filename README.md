SimEvo: A Simple Python Library for Simulations of Natural Selection
===

[![CI](https://github.com/YJack0000/SimEvo/actions/workflows/ci.yml/badge.svg)](https://img.shields.io/github/license/YJack0000/SimEvo)
![GitHub repo size](https://img.shields.io/github/repo-size/YJack0000/SimEvo)
[![PyPI - Implementation](https://img.shields.io/pypi/v/simevopy)](https://pypi.org/project/simevopy/)
[![CodeFactor](https://www.codefactor.io/repository/github/yjack0000/simevo/badge)](https://www.codefactor.io/repository/github/yjack0000/simevo)
![GitHub](https://img.shields.io/github/license/YJack0000/SimEvo)

[2024 Spring NSD - Final Project - SimEvo: A Simple Python Library for Simulations of Natural Selection](https://docs.google.com/presentation/d/1wtIAIIpjK2PZ31WvOYsF79wrXSUaM7NFj_LneUBSIJM/edit#slide=id.g2e20d1646bc_0_75)

## Overview

![Demo](https://github.com/YJack0000/SimEvo/assets/49984479/91d01cc1-6bad-45c1-8785-5fed11814140)

> **Project Overview:** Simulating Natural Selection in Ecosystems using C++ and Python

SimEvo leverages the computational efficiency of C++ for the core simulation engine and offers an intuitive Python interface for ease of use and accessibility. It models ecosystems composed of organisms with genetically determined traits, interacting within an environment that includes resources like food. The simulation observes the principles of natural selection, with organism survival and reproduction influenced by their traits and environmental factors.

- 📂 **GitHub Repository**: [https://github.com/YJack0000/SimEvo](https://github.com/YJack0000/SimEvo)
- 💻 **Implemented In**:
  - **Core Library**: C++
  - **Bindings & Interface**: Python

## How to use

### Install directly from PyPI
!!!This feature is only support Linux and MacOS currently. 
```
pip install simevopy
```

### Build from scratch

#### Build for testing
```
mkdir build; cd build
cmake -S .. -B . -DBUILD_TESTS=true
make
ctest -V
```
#### Build for python package
```
python setup.py install
```


### Components:

SimEvo integrates a high-performance C++ simulation engine with Python bindings for ease of use. Key architectural components include:

* **Core Simulation Engine (C++)**: Manages the dynamics of natural selection and organism interactions.
* **Python API (C++/Python via pybind11)**: Allows users to define and manipulate the simulation parameters easily.
* **Spatial Indexing (Quadtree)**: Optimizes computational tasks related to spatial relationships and interactions among entities.

### Spatial Indexing Data Structure

Spatial indexing is crucial for efficiently managing the interactions within the simulation. SimEvo uses a Quadtree for spatial indexing to enhance performance significantly:

* **Default Implementation (Brute-force)**: Suitable for simulations with fewer entities.
* **Optimized Implementation (Quadtree)**: Ideal for dense simulations, significantly reducing computational overhead and improving interaction efficiency.

### Default and Optimized Implementations

**Default Implementation**: In the initial stages or in simulations where the density of entities is low, a brute-force approach is employed. This method involves calculating the distances between all pairs of objects to determine proximity-based interactions. While straightforward, this approach is computationally intensive as the number of entities increases.

**Optimized Implementation**: For more complex simulations with a higher density of entities, SimSel utilizes a Quadtree data structure for spatial indexing. Quadtrees are highly efficient for 2D spatial data, allowing for rapid queries of objects within specific areas. This optimization significantly reduces the computational cost of distance calculations and interaction determinations, making the simulation more scalable and performant.

## API Description

The SimSel library offers a Python API designed for simplicity and ease of use, enabling users to simulate and study natural selection dynamics within virtual ecosystems. Here's an overview of the key components and functionalities provided by the API:

### Environment Class

- **Purpose**: Represents the simulation's environment, including dimensions and conditions.
- **Initialization**:
  - `Environment(width, height, type="default")`: Creates a new environment with specified dimensions. The `type` parameter can be set to "optimize" for enhanced performance using advanced spatial indexing.
- **Methods**:
  - `add_organism(organism, x, y)`: Places an organism at specified coordinates within the environment.
  - `add_food(x, y)`: Adds a food resource at specified coordinates.
  - `simulate_iteration(steps)`: Advances the simulation by a given number of steps, allowing organisms to interact, consume food, and potentially reproduce.

### Organism Class

- **Purpose**: Models an individual organism within the ecosystem, including its genetic makeup.
- **Initialization**:
  - `Organism(genes)`: Creates a new organism with the specified `Genes` object.
- **Attributes**:
  - `genes`: The genetic makeup of the organism, influencing its traits and behaviors.

### Genes Class

- **Purpose**: Encapsulates the genetic information of organisms, dictating their characteristics and adaptability.
- **Initialization**:
  - `Genes(sequence)`: Constructs a `Genes` object with a genetic sequence represented as a string (e.g., "ABCD").

### Python Script Examples

The script sets up a SimSel simulation in a 500x500 space, initializing 10 "ABCD" gene organisms and distributing food to model resource scarcity. Operating in an "optimize" mode for efficiency, it cycles through 100 generations, each time scattering food and advancing the simulation to observe natural selection as organisms interact, feed, and reproduce based on genetic traits. You can find more examples in [examples](/examples). 

```python
from simevopy import Environment, Organism, Genes
import random

# Setup base organisms in the environment
def setup_base_organism(env, count=10):
    for _ in range(count):
        x = random.randint(0, env.width - 1)
        y = random.randint(0, env.height - 1)
        env.add_organism(Organism(Genes("ABCD")), x, y)

# Distribute food randomly within the environment
def distribute_food_randomly(env, food_count=50):
    for _ in range(food_count):
        x = random.randint(0, env.width - 1)
        y = random.randint(0, env.height - 1)
        env.add_food(x, y)

# Create an optimized environment
env = Environment(500, 500, type="optimize")

# Initialize organisms
setup_base_organism(env)

# Run the simulation over 100 generations
for i in range(100):
    print(f"Gen {i} th")
    distribute_food_randomly(env, 100)
    env.simulate_iteration(50)
```
