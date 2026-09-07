# Testing and Numerical Validation

## 1. Purpose

The `simtest` executable provides a numerical/integration validation harness for the simulation pipeline.

It is intentionally different from a conventional collection of small unit tests.

The test exercises the same `ExecuteSimStep()` path used by the interactive simulation and records intermediate fields for inspection.

## 2. Test Flow

```text
select pressure solver
        |
        v
initialize test fields
        |
        v
apply deterministic non-zero velocity condition
        |
        v
run ExecuteSimStep()
        |
        +--> divergence before projection
        +--> pressure
        +--> pressure residual
        +--> divergence after projection
        |
        v
write diagnostic CSV files
```

Each pressure solver is explicitly selected before its test run.

The default test resolutions are:

```text
8
16
32
64
128
256
```

The test can also accept user-specified resolutions.

## 3. Solvers

The harness exercises:

- Jacobi
- Gauss-Seidel
- Red-Black Gauss-Seidel

The purpose is to make the numerical behavior of the different iterative methods observable under the same simulation framework, not to claim that one method is universally superior.

## 4. Test Input

The test uses deterministic, non-zero velocity input rather than an entirely zero initial state.

This is important because a zero velocity field would trivially produce zero divergence and zero pressure, allowing every solver to appear successful without exercising meaningful numerical work.

The test applies a controlled velocity gradient and then performs the same boundary, divergence, pressure, and projection operations used by the application.

## 5. Diagnostics

The test can export CSV snapshots containing:

- pressure
- pressure residual
- divergence before projection
- divergence after projection

Output is written to a timestamped test-output directory.

The CSV files are diagnostic artifacts intended for inspection and comparison.

## 6. Acceptance and Failure Semantics

The test maintains numerical acceptance state across solver, resolution, and frame combinations.

A numerical acceptance failure does not immediately terminate the experiment. The test continues so that diagnostics for the remaining cases are still generated.

The test distinguishes:

- **numerical validation failure** — a numerical case does not satisfy its configured criterion
- **test execution failure** — the test machinery cannot complete, such as an output/infrastructure failure

The executable propagates execution failure through its process exit status so it can be used with CTest.

## 7. Observed Solver Behavior

A representative run used:

- 10 simulation frames
- 1250 pressure iterations
- resolutions from 8×8 through 256×256

Observed behavior:

| Solver | Coarse resolutions | Fine resolutions |
|---|---|---|
| Jacobi | Passes lower resolutions | Can fail the configured threshold |
| Gauss-Seidel | Passes | Passes |
| Red-Black Gauss-Seidel | Passes | Passes |

This is consistent with Jacobi's slower convergence on finer grids.

The result is useful numerical evidence rather than a reason to loosen the acceptance threshold simply to make every solver pass.

## 8. Example Diagnostic Result

An earlier 8×8, frame-0 run with 200 pressure iterations produced:

| Solver | Divergence before RMS | Divergence after RMS | Pressure residual RMS |
|---|---:|---:|---:|
| Jacobi | 2.516898e-1 | 6.647916e-5 | 6.647533e-5 |
| Gauss-Seidel | 2.516898e-1 | 6.008974e-7 | 6.144386e-7 |
| Red-Black Gauss-Seidel | 2.516898e-1 | 4.604429e-7 | 4.500723e-7 |

All three methods substantially reduce divergence in this case, while Gauss-Seidel and Red-Black Gauss-Seidel reach lower residual/divergence levels within the same iteration budget.

## 9. What This Test Is Not

This is **not** a fixed-RHS pressure-solver benchmark.

Each test frame executes the complete simulation step. Velocity can be advected and projected, and pressure can participate in subsequent steps.

Therefore, equal solver names, grids, and iteration counts do not necessarily mean that every method is solving an identical isolated linear system from an identical state.

A dedicated fixed-RHS solver benchmark would be a separate test.

## 10. CTest

A typical workflow is:

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

The standalone test executable can also be run directly when CSV diagnostics need to be inspected.

## 11. Clean-Clone Validation

The repository was validated from a fresh clone.

The validation included:

1. cloning the repository with submodules
2. initializing GLFW and Dear ImGui
3. configuring a new CMake build directory
4. building the project from scratch
5. launching the application
6. running the numerical test executable

This verified that the repository does not depend on the original development build directory or local dependency state.

## 12. Current Testing Limitations

The current harness is deliberately pragmatic rather than a complete numerical verification framework.

Limitations include:

- the primary solver comparison is performed through the full simulation step rather than a fixed linear-system benchmark
- CSV diagnostics provide more detail than the machine-checkable acceptance logic
- additional manufactured-solution tests could provide stronger isolated verification
- CFL/time-step behavior is not currently tested

The current test nevertheless exercises the shared numerical pipeline on non-trivial deterministic data and makes solver behavior directly inspectable.
