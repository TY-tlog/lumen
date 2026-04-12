# ADR-048: Hit-test via screen-space ray casting with spatial acceleration

## Status
Accepted (Phase 8)

## Context
Phase 3a's double-click editing paradigm extends to 3D. The user
double-clicks a 3D element and a property dialog opens. This
requires mapping a 2D screen pixel to a 3D scene element — ray
casting.

For Scatter3D with 1M+ markers, naive ray-sphere intersection per
marker is O(N) per click — too slow for interactive use.

## Decision
Each PlotItem3D implements `hitTestRay(Ray, maxDistance) →
optional<HitResult3D>`. The ray is computed from the screen pixel
via Camera::unproject.

**Per-type hit-testing**:
- **Scatter3D**: ray-sphere intersection. Accelerated by uniform
  spatial grid (cell size = 4× marker radius). Only check markers
  in grid cells the ray passes through. Expected: O(√N) per click.
- **Surface3D**: ray-triangle intersection on mesh. No spatial
  acceleration needed for typical grid sizes (256×256 = 131K
  triangles, fast enough via sequential check with early
  bounding-box reject).
- **VolumeItem**: hit-test not meaningful (volume is transparent
  compositing). Double-click anywhere in volume bounds opens
  dialog.
- **Streamlines**: ray-tube intersection (approximate as ray-
  cylinder per line segment).
- **Isosurface**: ray-triangle on extracted mesh (same as Surface3D).

**SpatialGrid3D for Scatter3D**:
Uniform grid in world space. Cell size = 4× marker radius (ensures
each marker is in at most 8 cells). Grid traversal via DDA (digital
differential analyzer) along the ray. Check all markers in traversed
cells. Return closest hit.

## Consequences
- + Interactive hit-test even for 1M+ markers
- + Per-type specialization: each type uses the most efficient test
- + SpatialGrid3D is simple to implement (~200 lines)
- + DDA traversal is a well-known algorithm
- - Spatial grid uses O(N) memory for marker cell assignments
- - Grid cell size heuristic may need tuning for very sparse or
  very dense distributions

## Alternatives considered
- **GPU FBO picking** (render each item in unique color ID, read
  pixel): elegant but requires an extra render pass, GPU→CPU
  round-trip latency (~5-10ms), and doesn't provide hit distance
  (needed for selecting the closest overlapping item). Rejected
  for primary hit-test; may be added as optimization for >10M
  items in a future phase.
- **CPU iteration without acceleration**: O(N) per click. Fine for
  <10K items but unacceptable for Scatter3D's 1M target. Rejected
  as the only path; kept as fallback for types with small element
  counts.
- **BVH (bounding volume hierarchy)**: O(log N) but more complex
  to build and maintain when data changes reactively. Deferred
  unless spatial grid proves insufficient.
