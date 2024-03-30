SimEvo: A Simple Python Library for Simulations of Natural Selection
---

## Basic Information

> **Project Overview:** Simulating Natural Selection in Ecosystems using C++ and Python

SimSel leverages the computational efficiency of C++ for the core simulation engine and offers an intuitive Python interface for ease of use and accessibility. It models ecosystems composed of organisms with genetically determined traits, interacting within an environment that includes resources like food. The simulation observes the principles of natural selection, with organism survival and reproduction influenced by their traits and environmental factors.

- 📂 **GitHub Repository**: [https://github.com/YJack0000/SimEvo](https://github.com/YJack0000/SimEvo)
- 💻 **Implemented In**:
  - **Core Library**: C++
  - **Bindings & Interface**: Python

## Problem to Solve
Understanding the dynamics of natural selection and ecosystem evolution involves complex interactions among organisms and their environments. SimSel aims to provide a simplified yet powerful simulation environment where users can define various organisms, their genetic makeup, and environmental factors, to observe how natural selection unfolds over generations.
In this context, the implementation of spatial indexing algorithms plays a critical role. Efficient spatial indexing is essential for managing the simulation's computational complexity, enabling the simulation to handle large numbers of organisms and their interactions within the environment accurately and swiftly. 

## Prospective Users

SimSel is aimed at a broad audience interested in evolutionary biology, including:
- **Educators and Students**: For teaching and learning evolutionary concepts through interactive simulations.
- **Researchers**: For conducting experiments on evolutionary strategies and hypotheses.
- **Hobbyists**: For exploring the principles of evolution in a virtual environment.

## System Architecture
SimSel comprises a high-performance simulation engine written in C++ for handling the complex computations of natural selection dynamics efficiently. Python bindings, created using pybind11, make this engine accessible to users who can interact with the simulation using a simple Python API.

### Components:

* **Genetic Traits and Organism Definition (C++)**: Models organisms with specific genetic traits that affect their survival and reproduction.
* **Environmental Model (C++)**: A dynamic model of the environment, including resources like food, which organisms interact with.
* **Simulation Engine (C++)**: The core component that runs the simulation, applying rules of natural selection and tracking the evolution of populations over time.
* **Python API (C++ and Python via pybind11)**: Provides a user-friendly interface for defining simulations, including organisms, their genes, and environmental settings.

### Spatial Indexing Data Structure

Spatial indexing is critical in simulations of natural environments, where the efficient querying of organism positions and interactions can drastically reduce computational overhead. By indexing the spatial data, SimSel can quickly perform operations such as locating nearby food sources for organisms or detecting potential mates within a specified radius.

#### Abstraction Layer
SimSel introduces an abstraction layer for spatial indexing, encapsulated by the interfaces `ISpatialIndex` and `ISpatialObject`. This design allows for future expansions or modifications to the underlying spatial data structures without necessitating changes in the core simulation logic.

**ISpatialIndex Interface**: Defines the methods for adding, removing, and querying spatial objects. This interface is pivotal for integrating different spatial data structures according to the simulation's needs.

**ISpatialObject Interface**: Represents any entity within the simulation environment that occupies space, such as organisms and food items. This interface ensures that the spatial index can manage a variety of object types, making the simulation more versatile.

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

The script sets up a SimSel simulation in a 500x500 space, initializing 10 "ABCD" gene organisms and distributing food to model resource scarcity. Operating in an "optimize" mode for efficiency, it cycles through 100 generations, each time scattering food and advancing the simulation to observe natural selection as organisms interact, feed, and reproduce based on genetic traits.

```python
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
```

## Engineering Infrastructure

- **Automatic Build System**:
    - [GNU make](https://www.gnu.org/software/make/manual/make.html)
- **Version Control**:
    - Git
    - Github
- **Testing Framework**:
    - C++: [Google Test](https://github.com/google/googletest)
    - Python: [pytest](https://docs.pytest.org/en/7.4.x/)
- **Documentation**:
    - Markdown
- **Continuous Integration**
  - [Github Actions](https://github.com/features/actions)

## Schedule

Week 1 (4/8):
- Set up and model the basic C++ environment.
- Studying the relevant field knowledge and adjust the existing interface accordingly.
- Set up a CI pipeline for unit testing.

Week 2 (4/15):
- Implement genetic traits and organism dynamics.
- Develop pybind11 wrappers.
- Write unit tests for both implementations.

Week 3 (4/22):
- Implement the Environment with detailed simulation logic.
- Implement the DefaultSpatialIndex and possibly adjust some interfaces.
- Develop pybind11 wrappers.
- Write unit tests for the implementations.

Week 4 (4/29):
- Allocate this week as flexible time to address potential issues from last week's development.

Week 5 (5/6):
- Implement a higher performance data structure, currently planned to be Quadtree.
- Write unit tests for the new implementation.

Week 6 (5/13):
- Set up a CI pipeline for performance evaluation.
- Create documentation and update example scripts.

Week 7 (5/20):
- Prepare for the paper presentation.

Week 8 (5/27):
- Designate as flexible time for any overflow tasks or final adjustments.
