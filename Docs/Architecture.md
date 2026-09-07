# Architecture

## 1. Overview

The project separates field representation, numerical operations, simulation orchestration, visualization, and testing.

```text
Field representation
        |
        v
Numerical operators
        |
        v
Simulation-step orchestration
        |
        +------------------+
        |                  |
        v                  v
Numerical diagnostics   Visualization
```

The main architectural goal is to keep the field container independent of any particular simulation domain.

## 2. `Field2D<T>`

`Field2D<T>` is the generic two-dimensional field representation.

It provides grid dimensions, data storage, indexed access, iteration/access helpers, ghost-cell storage, and interpolation support where appropriate.

The container does not decide whether a field represents density, pressure, temperature, divergence, or another quantity. Simulation-specific meaning remains outside the storage type.

## 3. Ghost Cells

The simulation uses a layer of ghost cells around the physical domain.

Ghost cells provide storage for boundary values and allow finite-difference stencils to operate near the boundary without embedding boundary policy into `Field2D<T>`.

Boundary treatment is therefore an explicit numerical operation.

## 4. MAC Velocity Grid

Scalar quantities are cell-centred while velocity is stored on a staggered MAC grid.

Conceptually:

```text
            v-face
               |
               |
      u-face -- p -- u-face
               |
               |
            v-face
```

For a cell `(i,j)`:

- `u` is stored on vertical faces
- `v` is stored on horizontal faces
- pressure is stored at the cell centre

This makes face-based divergence and pressure-gradient calculations natural and provides a direct representation of the velocities crossing each cell boundary.

## 5. Numerical Operator Layer

Numerical transformations are exposed through the simulation-operator layer.

Representative operations include:

- source/injection operations
- advection
- divergence
- differential operators
- pressure solving
- residual computation
- pressure gradient
- velocity projection
- diagnostics

The operators transform fields rather than owning the simulation state.

This is the central reason `Field2D<T>` can remain domain-independent.

## 6. Simulation Pipeline

The application uses a single numerical-step entry point:

```cpp
sim_ops::ExecuteSimStep(...)
```

The current pipeline is conceptually:

```text
velocity/source update
        |
        v
velocity advection
        |
        v
boundary treatment
        |
        v
divergence
        |
        v
pressure solve
        |
        v
pressure residual
        |
        v
velocity projection
        |
        v
boundary treatment / diagnostics
        |
        v
density advection
```

Configuration controls which optional numerical operations are active.

Centralizing the sequence prevents the interactive application and test harness from maintaining separate copies of the numerical pipeline.

## 7. Pressure Solvers

The pressure solve is selected through `SimulationConfig`.

The implementation provides:

- Jacobi
- Gauss-Seidel
- Red-Black Gauss-Seidel

The surrounding simulation pipeline treats the solver as an interchangeable numerical operation.

This allows the same simulation-step implementation to be exercised with different iterative methods.

## 8. Simulation Ownership

`Simulation` owns the application-facing simulation state, including the active velocity, divergence, pressure, residual, scalar/transient fields, configuration, and interaction state.

It coordinates the resources while numerical transformations are delegated to `sim_ops`.

## 9. Rendering

The numerical simulation is independent of the renderer.

The current renderer uses GLFW/OpenGL, GLAD, an off-screen framebuffer, fullscreen shader rendering, dynamic line rendering, and Dear ImGui.

Scalar visualization maps a selected scalar field into a texture displayed in the GUI viewport.

Velocity visualization can represent either cell-centred samples or the underlying face-centred MAC velocity.

Ghost cells extend the computational grid beyond the physical domain and are displayed in the simulation editor for debugging and inspection.

## 10. Interaction

Mouse input passes through:

```text
Window coordinates
        |
        v
Viewport/image coordinates
        |
        v
Normalized/render coordinates
        |
        v
Simulation grid coordinates
```

The grid-space interaction system supports velocity injection, with the same coordinate mapping applicable to other field types.

UI coordinate handling remains separate from the numerical operators.

## 11. Testing Architecture

The `simtest` target uses the same numerical operations as the application:

```text
simtest/test_main.cpp
        |
        v
SimTest
        |
        v
PressureSolverTest()
        |
        v
ExecuteSimStep()
        |
        +--> diagnostics
        |
        v
CSV output
```

This makes the test an integration exercise of the actual numerical path rather than a duplicate miniature solver.

The tradeoff is that it is not an isolated pressure-solver benchmark; the full simulation step can alter the state between frames.

## 12. Architectural Rationale

The project deliberately avoids introducing a general-purpose simulation graph or resource-management framework.

The current decomposition is:

```text
Field2D<T>
    = data

sim_ops
    = numerical transformations

Simulation
    = state ownership + orchestration

Renderer
    = visualization

GUI
    = interaction/configuration

SimTest
    = numerical diagnostics
```

This keeps the code explicit and the numerical sequence easy to follow. Additional abstraction would only be justified if the project grows to contain multiple independent pipelines or resource-lifetime problems that benefit from it.
