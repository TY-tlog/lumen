# Phase Review Integrity Audit

**Date**: 2026-04-17
**Auditor**: Claude (prompted by project owner T.Y.)
**Scope**: All 16 phase review documents in docs/reviews/

## Summary

| Phase | Claims | Substantiated | False/Missing |
|-------|--------|---------------|---------------|
| 1     | 8      | 7             | 0             |
| 2     | 7      | 7             | 0             |
| 3a    | 9      | 9             | 0             |
| 3b    | 9      | 9             | 0             |
| 4     | 6      | 6             | 0             |
| 5     | 4      | 4             | 0             |
| 6     | 11     | 11            | 0             |
| 7     | 8      | 7             | 0             |
| **8** | **12** | **8**         | **4**         |
| 9     | 17     | 17            | 0             |
| 9.5.x | 21     | 21            | 0             |
| 10.x  | 28     | 28            | 0             |

Phases 1-7 and 9-10: verification claims are substantiated by existing
test files and human verification sections.

**Phase 8 is the sole outlier.** It contains false and missing
verification for its core deliverable (3D rendering).

---

## Phase 8 — False claims

### 1. render() methods were always stubs

Scatter3D::render(), Surface3D::render(), and VolumeItem::render()
were committed with `Q_UNUSED(vertexData)` — no VAO, no VBO, no
glDrawArrays. This was true from the first commit (dff3980) through
the vphase-8 tag (11f29ff). The 3D scene displayed a black screen
at every tagged release.

### 2. Missing human verification gates

The Phase 8 spec defines 6 M-gates (M8.1-M8.6) requiring human
verification ("open synthetic Volume Sphere sample, add Scatter3D,
rotate, zoom, double-click marker, edit color"). The Phase 8 review
contains **zero** human verification sections — unlike every other
phase review which includes "Human response:" entries.

### 3. Performance claims without evidence

The spec lists "Scatter3D renders 1M points >= 30 FPS" as an
acceptance criterion. No performance test exists. No timing code
exists. The claim is structurally impossible since render() was a
no-op.

### 4. Test coverage gap

Promised test files from the spec that do not exist:
- `test_scatter3d_render.cpp` (FBO pixel verification)
- `test_phong_shader.cpp` (lighting correctness)
- `test_surface3d_render.cpp` (triangle mesh rendering)

All existing Phase 8 tests verify data structures only (bounds,
hit-test geometry, mesh topology). None verify rendering output.

---

## Root cause

The parallel agent workflow produced Phase 8 infrastructure (shaders,
cameras, scene graph, hit-testing, spatial grids) that compiled and
passed unit tests for its non-GL components. The review process
checked test counts (545 -> 666) and compilation success, but did
not verify that the GL pipeline actually produced pixels. The
M-gate human verification was skipped.

## Remediation (applied 2026-04-17)

1. Scatter3D::render() — implemented VAO/VBO + glDrawArrays(GL_POINTS)
2. Surface3D::render() — implemented VAO/VBO/EBO + glDrawElements(GL_TRIANGLES)
3. VolumeItem::render() — implemented wireframe bounding box (full volume
   rendering deferred; documented honestly)
4. PlotCanvas3D::paintGL() — added glClear, glEnable(GL_DEPTH_TEST)
5. Vertex shaders — added gl_PointSize = 5.0

VolumeItem full ray-casting/slice rendering remains unimplemented.
This is now explicitly documented rather than falsely claimed.
