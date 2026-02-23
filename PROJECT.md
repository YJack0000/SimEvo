# PROJECT.md — SimEvo Architecture & Design

This document describes the technical architecture of SimEvo for developers and AI assistants working on the codebase.

## Overview

SimEvo is a C++20 simulation engine for natural selection with Python bindings via pybind11. The C++ core handles performance-critical simulation (spatial indexing, iteration loop, object lifecycle), while Python provides the user-facing API for defining organisms, behaviors, and running experiments.

**Package:** `simevopy` (PyPI)
**License:** MIT

## System Architecture

```
┌──────────────────────────────────────────────────────┐
│                    Python Layer                       │
│   User scripts, custom behaviors, experiment setup    │
├──────────────────────────────────────────────────────┤
│                  pybind11 Bindings                     │
│   bindings/core/*_bindings.cpp                        │
│   snake_case API, py::arg(), docstrings               │
├──────────────────────────────────────────────────────┤
│                   C++ Engine                           │
│   Environment → ISpatialIndex → Objects               │
│   Simulation loop, thread management, profiling       │
└──────────────────────────────────────────────────────┘
```

## Core Classes

### EnvironmentObject (base class)
- Every object in the simulation derives from this
- Has a unique `boost::uuids::uuid` and a 2D position
- Virtual `postIteration()` for per-tick lifecycle

### Organism
- Derives from EnvironmentObject
- **Genes**: 4-byte DNA sequence; each byte maps to a trait (speed, size, awareness, reserved). Each byte 0-255 is divided by 4 to get the attribute value (0-64 range)
- **Lifespan**: starts at 500, consumed each tick based on speed/size/awareness. Dies when <= 0
- **Reproduction**: when lifespan > 1000, can create offspring with mutated genes. Parent's lifespan is halved
- **Behavior strategies**: injectable `ReactionStrategy` and `InteractionStrategy` via `std::function`. Defaults to built-in C++ behavior if not set. Custom strategies (from Python or C++) are inherited by offspring

### Food
- Derives from EnvironmentObject
- Has a FoodState (FRESH/EATEN) and an energy value (default 500)
- Once eaten, marked EATEN and removed during cleanup

### Environment
- The simulation world: owns all objects via `unordered_map<uuid, shared_ptr<EnvironmentObject>>`
- Spatial queries delegated to `ISpatialIndex<uuid>`
- Constructor: `Environment(width, height, type="default"|"optimized", numThreads=1)`

### ISpatialIndex
- Interface for spatial queries (insert, remove, update, query-by-radius)
- **DefaultSpatialIndex**: brute-force O(n) per query
- **OptimizedSpatialIndex**: quadtree, better for large populations

## Simulation Loop

Each call to `simulateIteration(n)` runs `n` ticks. Each tick:

```
1. handleInteractions()
   - For each alive organism, query spatial index by SIZE radius
   - Call organism.interact() with nearby objects
   - Eats food (gains energy), kills smaller organisms (absorbs lifespan)
   - ⚠ Mutates shared state → must be single-threaded

2. handleReactions()
   - For each alive organism, query spatial index by REACTION radius (size + awareness)
   - Call organism.react() with nearby objects
   - Sets movement direction: flee from larger, chase smaller, approach food
   - ✅ Only writes to own fields → can be parallelized (when no Python callbacks)

3. postIteration()
   - Each object's postIteration() is called
   - Organisms: deduct life consumption, make movement, update position
   - Positions clamped to environment bounds
   - Spatial index positions updated

4. cleanUp() (after all ticks)
   - Remove dead organisms (store in deadOrganisms list)
   - Remove eaten food (increment foodConsumption counter)
```

## Behavior Strategy System

Organisms use a strategy pattern for customizable behavior:

```
ReactionStrategy = std::function<pair<float,float>(Organism&, vector<shared_ptr<EnvironmentObject>>)>
InteractionStrategy = std::function<void(Organism&, vector<shared_ptr<EnvironmentObject>>)>
```

- If no custom strategy is set, `defaultReaction()` / `defaultInteraction()` are used
- Custom strategies are set via `setReactionStrategy()` / `setInteractionStrategy()`
- Strategies propagate to offspring during `reproduce()`
- Python can define these as regular Python functions via pybind11

## Genes & Mutation

DNA is a 4-byte array. Default mutation adds a uniform random offset in [-3, +3] to each byte per generation. Custom mutation functions can be provided via `Genes::MutationFunction`.

Trait mapping:
| DNA Index | Trait      | Range (raw) | Range (scaled) |
|-----------|------------|-------------|----------------|
| 0         | Speed      | 0-255       | 0.0 - 63.75   |
| 1         | Size       | 0-255       | 0.0 - 63.75   |
| 2         | Awareness  | 0-255       | 0.0 - 63.75   |
| 3         | Reserved   | 0-255       | —              |

## Life Consumption Formula

Default formula (can be overridden with `LifeConsumptionCalculator`):

```
consumption = (speed/10)^2 + (size/10)^2 * (size/15) + (awareness/10)) * 1.3
```

Larger, faster organisms burn energy disproportionately more.

## Thread Safety Model

| Phase            | Thread Safety | Reason |
|------------------|---------------|--------|
| handleInteractions | Single-threaded | Mutates food state, organism lifespans |
| handleReactions    | Parallelizable* | Each organism only writes to its own movement |
| postIteration      | Single-threaded | Updates shared spatial index |

*When Python callbacks are set as strategies, reactions must also be single-threaded to avoid GIL deadlocks. The main thread holds the GIL; worker threads cannot acquire it.

## File Structure

```
include/
  core/
    EnvironmentObject.hpp    # Base class (UUID + position)
    Organism.hpp             # Living entity with genes and strategies
    Food.hpp                 # Consumable energy source
    Genes.hpp                # 4-byte DNA with mutation
    Environment.hpp          # Simulation world and loop
  index/
    ISpatialIndex.hpp        # Spatial query interface
    DefaultSpatialIndex.hpp  # Brute-force implementation
    OptimizedSpatialIndex.hpp # Quadtree implementation
  utils/
    profiler.hpp             # Performance timing utility

src/core/                    # Implementation files
src/index/                   # Spatial index implementations

bindings/
  python_bindings.cpp        # PYBIND11_MODULE entry point
  core/*_bindings.cpp        # Per-class binding definitions

tests/
  cpp/                       # Google Test C++ tests
  python/                    # pytest Python tests

examples/                    # Runnable Python example scripts
```

## Build System

- **CMake 3.24+** with C++20 required
- `BUILD_BINDINGS=ON` (default): builds pybind11 Python module
- `BUILD_TESTS=ON`: builds Google Test C++ tests
- `setup.py` wraps CMake for `pip install .`
- CI: GitHub Actions runs both C++ and Python tests on every PR

## Dependencies

- **Boost** (uuid only): object identification
- **pybind11**: Python bindings
- **Google Test**: C++ tests (fetched automatically via CMake)
