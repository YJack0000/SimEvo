# CLAUDE.md — SimEvo Project Guide

## Project Overview

SimEvo is a C++ simulation evolution engine with Python bindings (pybind11). The C++ engine handles the environment, spatial indexing, and simulation loop. Python users define organisms, behaviors, and interactions without modifying the C++ core.

**Package name:** `simevopy` (published to PyPI)

## Build & Run

```bash
# Build C++ library + Python bindings
cmake -S . -B build -DBUILD_BINDINGS=ON
cmake --build build

# Build with C++ tests
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_BINDINGS=OFF
cmake --build build
cd build && ctest -V

# Install Python package (builds via CMake internally)
pip install .

# Run Python tests
pytest

# Run a single Python test
pytest tests/python/test_chasing.py -v
```

**Dependencies:** CMake 3.24+, C++20 compiler, Boost (uuid), pybind11, Google Test (fetched automatically for C++ tests)

## Project Structure

```
include/
  core/           # Core simulation headers
    Environment.hpp, EnvironmentObject.hpp, Food.hpp, Genes.hpp, Organism.hpp
  index/          # Spatial index interface + implementations
    ISpatialIndex.hpp, DefaultSpatialIndex.hpp, OptimizedSpatialIndex.hpp
  utils/          # Utilities (profiler.hpp)
src/
  core/           # Core simulation implementation
  index/          # Spatial index implementations
bindings/
  python_bindings.cpp          # Module entry point (PYBIND11_MODULE)
  core/                        # Per-class pybind11 binding files
    Environment_bindings.cpp, Organism_bindings.cpp, Food_bindings.cpp, etc.
tests/
  cpp/            # Google Test C++ tests
  python/         # pytest Python tests
examples/         # Python example scripts
```

## Architecture

- **EnvironmentObject** — base class for everything placed in the environment (has UUID, position)
- **Organism** — extends EnvironmentObject; has genes, lifespan, behavior strategies
- **Food** — extends EnvironmentObject; can be eaten for energy
- **Environment** — owns a spatial index, runs the simulation loop (reactions → interactions → cleanup)
- **ISpatialIndex** — interface for spatial queries; `DefaultSpatialIndex` (brute-force) and `OptimizedSpatialIndex` (quadtree)

Simulation loop per iteration: `updatePositionsInSpatialIndex()` → `handleReactions()` → `handleInteractions()` → `postIteration()` → `cleanUp()`

## Coding Conventions

- **C++20** standard required
- **Clang-format**: Google style, 4-space indent, 100 column limit (see `.clang-format`)
- **Doxygen comments**: Use `@brief`, `@param`, `@return` for all public API methods. Keep these even when refactoring — they will be used for documentation generation.
- **Header guards**: `#ifndef CLASSNAME_HPP` / `#define` / `#endif`
- **Smart pointers**: Use `std::shared_ptr` for objects stored in the environment
- **Naming**: PascalCase for classes, camelCase for methods/variables in C++; snake_case for Python bindings

## Python Bindings (pybind11)

- Each C++ class has a corresponding `*_bindings.cpp` file in `bindings/core/`
- Forward-declared in `bindings/python_bindings.cpp` and called via `init_ClassName(m)`
- Python method names use snake_case (e.g., `get_position`, `add_organism`)
- Use `py::arg(...)` for all parameters in bindings
- Add docstrings to all bound methods
- Exception registrations go in `python_bindings.cpp` (not duplicated in binding files)

## Branching & PR Conventions

- **Main branch:** `main`
- **Branch naming:** `fix/description`, `feat/description`, `refactor/description`, `ci/description`
- **PR titles:** concise, imperative mood
- **Commit messages:** conventional commits style (`feat:`, `fix:`, `refactor:`, `ci:`, `chore:`)
- CI runs on all PRs: C++ tests (ctest) + Python tests (pytest)
- Version bumping and PyPI publishing happen automatically on merge to main

## Testing

- **C++ tests:** Google Test, located in `tests/cpp/`. Built when `-DBUILD_TESTS=ON`.
- **Python tests:** pytest, located in `tests/python/`. Run after `pip install .`.
- Always run both C++ and Python tests before opening a PR.

## Key Gotchas

- **Thread safety:** The interaction phase mutates shared state (organism lifespan, food state) and must run single-threaded. The reaction phase can be parallelized.
- **GIL deadlock:** When Python callbacks (behavior strategies) are used, worker threads cannot acquire the GIL if the main thread holds it. Keep simulation single-threaded when Python callbacks are involved.
- **pybind11 copyability:** Adding `std::atomic` or `std::mutex` to bound classes makes them non-copyable, which breaks pybind11's default return value policy. Avoid this.
- **Spatial index:** Environment constructor accepts `type` param: `"default"` (brute-force) or `"optimized"` (quadtree).
