# Numerical Methods

## 1. Simulation Model

The current solver is an Eulerian, grid-based simulation combining:

- a staggered MAC velocity field
- cell-centred scalar quantities
- incompressibility enforced through pressure projection
- semi-Lagrangian advection of the scalar density field

The current model is best described as:

```text
prescribed/interactive velocity
        |
        v
incompressible velocity field
        |
        v
passive scalar transport
```

It is not a complete Navier-Stokes smoke model.

## 2. Field Layout

### Scalar fields

Pressure, divergence, residual, and passive scalar quantities are represented at cell centres.

### Velocity

Velocity uses a staggered MAC layout:

- `u` is stored on vertical cell faces
- `v` is stored on horizontal cell faces
- pressure is stored at cell centres

This gives the divergence operator direct access to the velocities crossing each cell face.

## 3. Ghost Cells and Boundary Conditions

A layer of ghost cells surrounds the physical grid.

The ghost layer supports finite-difference operations near the boundary and provides explicit boundary treatment.

The pressure formulation uses Neumann-style pressure boundary handling associated with no-flux velocity boundaries.

Boundary-condition operations are provided by `Field2D<T>` and invoked by the numerical pipeline at the appropriate stages of the simulation step.

## 4. Divergence

For an incompressible velocity field, the target constraint is:

$$
\nabla \cdot \mathbf{u} = 0
$$

On the MAC grid, divergence is computed from opposing face velocities:

$$
\nabla \cdot \mathbf{u}
\approx
\frac{u_{i+1/2,j}-u_{i-1/2,j}}{\Delta x}
+
\frac{v_{i,j+1/2}-v_{i,j-1/2}}{\Delta y}
$$

The resulting divergence field provides the right-hand side for the pressure solve.

## 5. Pressure Projection

After source updates and velocity advection, the velocity field will generally not satisfy the incompressibility constraint.

A pressure field is therefore computed and its gradient is subtracted from the intermediate velocity:

$$
\mathbf{u}^{n+1}
=
\mathbf{u}^{*}
-
\nabla p
$$

with the exact discrete scaling determined by the implementation.

The purpose of projection is to reduce the discrete divergence of the velocity field.

The test harness records divergence before and after projection.

## 6. Pressure Poisson Equation

Pressure is obtained from a discrete Poisson equation derived from the incompressibility constraint.

Conceptually:

$$
\nabla^2 p = \nabla \cdot \mathbf{u}^{*}
$$

with sign and scaling determined by the discrete convention used in the implementation.

The equation is solved iteratively.

## 7. Iterative Pressure Solvers

### Jacobi

Jacobi computes each update from values belonging to the previous iteration.

It is simple but converges relatively slowly. In the project's numerical tests, it can satisfy the configured criterion on coarse grids while failing it at finer resolutions under the same iteration budget.

### Gauss-Seidel

Gauss-Seidel uses newly updated neighboring values within an iteration.

This generally improves convergence compared with Jacobi for an equal iteration count.

### Red-Black Gauss-Seidel

Red-Black Gauss-Seidel partitions cells into two alternating sets and updates one set before the other.

This preserves the useful convergence characteristics of Gauss-Seidel while exposing a more regular update pattern.

## 8. Pressure Residual

The solver computes a pressure-equation residual field.

Conceptually:

$$
r = b - Ap
$$

where:

- $A$ is the discrete Poisson operator
- $p$ is the current pressure field
- $b$ is the divergence-derived right-hand side

Residual values are exported by the test harness for inspection.

## 9. Semi-Lagrangian Advection

Scalar transport uses a semi-Lagrangian approach.

Instead of directly moving a scalar value from one cell to another, the method traces a characteristic backwards from the destination position.

```text
destination position
        |
        v
backtrace through velocity
        |
        v
sample previous scalar field
        |
        v
write reconstructed value
```

The backtraced position generally does not coincide with a grid point, so the previous field must be sampled continuously.

## 10. Bilinear Interpolation

Scalar values at arbitrary positions are reconstructed with bilinear interpolation using the four surrounding grid samples.

Interpolation is therefore part of the numerical transport scheme, not merely a rendering technique.

The renderer also uses linear texture filtering for smoother visualization, but that visual filtering is separate from the numerical interpolation.

## 11. Velocity Sampling

Because the velocity field is staggered, sampling velocity at arbitrary positions requires combining/interpolating the face-centred components at the requested location.

This permits characteristic backtracing while preserving the MAC storage layout.

## 12. Numerical Step

The shared simulation operation can be summarized as:

```text
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
pressure projection
        |
        v
boundary treatment
        |
        v
diagnostics
        |
        v
density advection
```

Velocity and source/injection operations are applied as part of the simulation state setup and interaction before the numerical step.

The exact optional operations are controlled by configuration.

## 13. Numerical Scope

The current implementation does not include:

- viscosity
- buoyancy
- a complete external-force model
- vorticity confinement
- adaptive CFL-based substepping
- GPU compute-based numerical updates

The project therefore focuses on discrete field representation, transport, incompressibility, pressure solving, and the software architecture supporting them.
