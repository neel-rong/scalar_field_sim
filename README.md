# Scalar Field Simulation

A C++20/OpenGL 2D Eulerian simulation project focused on numerical methods, field representations, and simulation-oriented software architecture.

The project implements an incompressible-flow pipeline with a staggered MAC velocity grid and passive scalar transport. It is intentionally scoped as a technical simulation project rather than a complete smoke or production fluid solver.

## Overview

The simulation separates **field data representation**, **numerical operators**, **simulation orchestration**, and **visualization**.

```text
Application
    |
    v
Simulation
    |
    v
sim_ops::ExecuteSimStep(...)
    |
    +--> velocity advection
    +--> velocity boundary conditions
    +--> divergence
    +--> pressure solve
    +--> pressure residual
    +--> pressure projection
    +--> velocity boundary conditions
    +--> post-projection diagnostics
    +--> density advection
    |
    v
Renderer / Debug Visualization
```

The numerical work is CPU-based. OpenGL is used for visualization and interactive inspection of the simulation state.

## Features

- Generic `Field2D<T>` grid container
- C++20 implementation
- One layer of ghost cells
- Cell-centred scalar fields
- Staggered MAC velocity field
- Semi-Lagrangian advection with bilinear interpolation
- Divergence computation
- Pressure Poisson solve
- Jacobi, Gauss-Seidel, and Red-Black Gauss-Seidel pressure solvers
- Pressure residual diagnostics
- Pressure projection
- Configurable simulation and rendering settings
- Interactive mouse velocity injection
- OpenGL framebuffer-based rendering
- Scalar-field texture visualization
- Cell-centred and face-centred velocity visualization
- Numerical/integration test harness with CSV diagnostics

## Scope

The current simulation is best described as:

> **prescribed/interactive velocity source → incompressible flow projection → passive scalar transport**

It does not attempt to implement a complete Navier-Stokes smoke model.

The current scope does not include viscosity, buoyancy, a fully coupled force model, vorticity confinement, adaptive CFL substepping, or GPU compute simulation. These omissions are deliberate: the project concentrates on grid representation, discrete operators, pressure projection, iterative solvers, and the architecture around them.

## Architecture

The major separation is between **what data is stored** and **what numerical operation is performed on that data**.

`Field2D<T>` provides generic grid storage and access. Simulation semantics are not embedded in the container.

Scalar quantities such as density, pressure, divergence, and residual use cell-centred fields. Velocity uses a staggered MAC representation, with horizontal velocity on vertical faces, vertical velocity on horizontal faces, and pressure at cell centres.

Numerical transformations live in the simulation-operator layer. `Simulation` owns application-level state and the renderer visualizes it without owning numerical behavior.

The numerical step is centralized in:

```cpp
sim_ops::ExecuteSimStep(...)
```

so the interactive application and numerical test harness use the same simulation-step implementation.

## Project Structure

```text
scalar_field_sim/
├── include/
│   ├── field.h
│   ├── differential_operators.h
│   ├── grid_algorithms.h
│   ├── misc_operators.h
│   ├── sim_properties.h
│   ├── sim_solvers.h
│   ├── simulation.h
│   └── simulation_operators.h
├── src/
│   ├── main.cpp
│   ├── simulation.cpp
│   ├── simulation_operators.cpp
│   ├── renderer.cpp
│   └── GUI.cpp
├── simtest/
│   ├── sim_test.h
│   ├── sim_test.cpp
│   └── test_main.cpp
├── shaders/
├── third_party/
│   ├── glad/
│   ├── glfw/
│   └── imgui/
├── CMakeLists.txt
└── .gitignore
```

## Building

Clone with submodules:

```bash
git clone --recurse-submodules https://github.com/neel-rong/scalar_field_sim.git
cd scalar_field_sim
```

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build --config Release
```

The repository has been validated from a fresh clone, including successful initialization of the GLFW and Dear ImGui submodules and a clean build.

### Dependencies

- C++20 compiler
- CMake
- OpenGL 4.6-capable environment
- GLFW
- Dear ImGui
- GLAD

GLFW and Dear ImGui are Git submodules. GLAD is included directly under `third_party/glad`.

## Running

The main executable starts the interactive simulation/editor.

The GUI exposes simulation, emitter, rendering, viewport, and playback controls. The default configuration is intended to provide a useful starting visualization while remaining configurable for experimentation.

## Numerical Methods

See [`Docs/NumericalMethods.md`](Docs/NumericalMethods.md) for the detailed numerical description.

The main methods are:

- Eulerian grid representation
- MAC staggered velocity storage
- ghost-cell boundary treatment
- finite-difference operators
- semi-Lagrangian scalar advection
- bilinear interpolation
- pressure Poisson equation
- iterative pressure solvers
- pressure projection

## Testing

See [`Docs/Testing.md`](Docs/Testing.md) for the numerical test description.

The test harness uses deterministic non-zero velocity input and exercises the shared simulation-step pipeline for multiple pressure solvers, resolutions, and frames. It records pressure, residual, and divergence diagnostics to CSV.

It is best described as a **numerical/integration validation harness**, not as a fixed-RHS pressure-solver benchmark.

## Design Philosophy

- Keep field containers domain-independent.
- Keep numerical transformations separate from data ownership.
- Keep simulation orchestration centralized.
- Keep rendering independent of numerical behavior.
- Make numerical diagnostics inspectable independently of visualization.
- Prefer explicit, understandable architecture over abstraction without a current need.

## Limitations and Future Work

Potential future work includes explicit time-step handling and CFL-aware substepping, viscosity, buoyancy/force coupling, improved vorticity treatment, more isolated solver convergence tests, and GPU/compute-shader implementations.

These are future directions, not features of the current implementation.

## License

This project is licensed under the MIT License.

Third-party dependencies are distributed under their respective licenses.
See the relevant dependency directories for their license files.
