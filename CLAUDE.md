# CLAUDE.md — Development Guidelines for SimEvo

## Quick Reference

```bash
# Build and install Python package
pip install .

# Run all Python tests
pytest

# Build and run C++ tests
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_BINDINGS=OFF && cmake --build build && cd build && ctest -V
```

**Dependencies:** CMake 3.24+, C++20 compiler, Boost (uuid), pybind11

## Development Rules

### Always Test Before Opening a PR
- Run **both** C++ tests (`ctest`) and Python tests (`pytest`) before opening any PR
- If you add or change functionality, add corresponding tests
- If a test fails, fix it — don't skip or delete it

### New Features Must Have Example Code
- Every new user-facing feature (new API, new binding, new capability) must include an example script in `examples/`
- Examples should be self-contained and runnable: `python examples/your_example.py`
- This is non-negotiable — features without examples are incomplete

### Maintain Doxygen Documentation
- All public C++ APIs must have Doxygen comments: `@brief`, `@param`, `@return`, `@throws`
- When refactoring, **preserve existing Doxygen comments** — update them if the API changed, but never silently remove them
- These comments will be used for automated documentation generation
- Inline comments should explain **why**, not **what** — skip obvious ones

### Clean Architecture and C++ Best Practices
- Follow modern C++20 idioms and best practices
- Maintain separation of concerns: C++ engine handles simulation logic, Python defines behaviors
- Don't introduce unnecessary complexity — prefer simple, direct solutions
- Use `std::shared_ptr` for environment objects, `std::unique_ptr` for owned resources
- Avoid raw `new`/`delete`
- Clang-format: Google style, 4-space indent, 100 column limit (see `.clang-format`)

### Push Back When Needed
- If a requested change would violate clean architecture, degrade code quality, or introduce technical debt — **say so**
- Propose better alternatives rather than blindly following instructions
- The goal is to keep the project moving in a good direction, not just to complete tasks
- Flag potential issues in PR descriptions if you're unsure

## Project Architecture

```
include/core/     → C++ headers (Environment, Organism, Food, Genes, EnvironmentObject)
include/index/    → Spatial index interface (ISpatialIndex) + implementations
src/core/         → C++ implementation
bindings/core/    → pybind11 binding files (one per class)
tests/cpp/        → Google Test C++ tests
tests/python/     → pytest Python tests
examples/         → Python example scripts
```

- **EnvironmentObject** — base class with UUID + position
- **Organism** — has genes, lifespan, injectable behavior strategies (ReactionStrategy, InteractionStrategy)
- **Food** — consumable energy source
- **Environment** — owns spatial index, runs simulation loop: interactions → reactions → postIteration → cleanup
- **ISpatialIndex** — `"default"` (brute-force) or `"optimized"` (quadtree)

## Conventions

- **Naming:** PascalCase classes, camelCase C++ methods/vars, snake_case Python bindings
- **Branching:** `fix/`, `feat/`, `refactor/`, `ci/`, `docs/` prefixes
- **Commits:** conventional commits (`feat:`, `fix:`, `refactor:`, `ci:`, `docs:`)
- **Bindings:** each class gets `*_bindings.cpp` in `bindings/core/`, forward-declared in `python_bindings.cpp`
- **Bindings:** use `py::arg(...)` for all params, add docstrings to all bound methods
- **Exception registration:** only in `python_bindings.cpp`, not duplicated in binding files
- CI auto-bumps version and publishes to PyPI on merge to main

## Key Technical Gotchas

- **Thread safety:** Interaction phase mutates shared state → single-threaded. Reaction phase only writes to each organism's own fields → parallelizable.
- **GIL deadlock:** Python callbacks from worker threads can't acquire GIL if main thread holds it. Keep simulation single-threaded when Python strategies are set.
- **pybind11 copyability:** `std::atomic` or `std::mutex` on bound classes makes them non-copyable, breaking pybind11. Avoid this.
