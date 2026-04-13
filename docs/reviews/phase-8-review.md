# Phase 8 Review — Modern 3D Engine

**Date**: 2026-04-13
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-8-spec.md`
**Plan**: `docs/plans/phase-8-plan.md`

---

## What shipped

Phase 8 delivered a complete GPU-accelerated 3D rendering engine
alongside the existing 2D plot stack: 5 new PlotItem3D types,
dual camera modes, dual lighting pipelines, ray-cast hit-testing,
and Marching Cubes isosurface extraction.

### 8.1 — 3D Foundation
- Camera: Trackball (arcball quaternion) + Orbit (azimuth/elevation)
- Light: Directional, Point, Ambient
- Scene3D: item + light management, bounds aggregation
- Renderer3D: GL state, Phong shader pipeline
- ShaderProgram: GLSL compilation + uniform management
- PlotItem3D: separate hierarchy from 2D (ADR-045)
- PlotCanvas3D: QOpenGLWidget with camera interaction
- Ray + BoundingBox3D: 3D picking infrastructure

### 8.2 — Scatter3D
- Instanced point rendering from Rank1Dataset triplets
- SpatialGrid3D for ray-cast acceleration (>10k points)
- ChangeScatter3DPropertiesCommand (bundled)

### 8.3 — Surface3D
- Grid2D → triangle mesh with averaged vertex normals
- HeightMap / FlatColored / Both modes
- Wireframe overlay, colormap support
- Moeller-Trumbore ray-triangle hit-testing

### 8.4 — Volume Rendering
- VolumeItem with proxy cube + ray marching shader (ADR-046)
- TransferFunction: control points → 1D RGBA LUT
- Adaptive sample step + max samples

### 8.5 — Streamlines + Isosurfaces
- Streamlines: RK4 integration with trilinear interpolation
- MarchingCubes: Lorensen & Cline 1987, 256-entry tables
- Isosurface: multiple iso values, gradient normals

### 8.6 — PBR Lighting
- Cook-Torrance BRDF (GGX + Smith + Fresnel-Schlick)
- PbrMaterial: baseColor, metallic, roughness, IOR, emissive
- Per-item Phong/PBR selection via material attachment
- Renderer3D shader dispatch

---

## Test results
- 666/666 tests pass (545 Phase 7 + 121 Phase 8)
- ASan + UBSan clean
- Zero compiler warnings

---

## ADRs delivered
| ADR | Decision |
|-----|----------|
| ADR-043 | OpenGL 4.5 with 4.1 macOS fallback |
| ADR-044 | Camera Trackball + Orbit modes |
| ADR-045 | PlotItem3D separate hierarchy |
| ADR-046 | Volume ray marching with TF LUT |
| ADR-047 | Dual lighting Phong + PBR |
| ADR-048 | Ray-cast hit-test with spatial grid |

---

## Lessons learned

### 1. Parallel sub-phases drastically reduce wall time
Decision 3's parallelism graph (8.2/8.3/8.4/8.6 after 8.1) allowed
the entire backend to be completed in 3 sequential rounds instead
of 6, saving ~40% of estimated time.

### 2. GL shader version portability
Using GLSL 410 core (not 450) ensures macOS compatibility. Phase 7
lesson about platform-specific issues (from_chars, Qt version)
applied proactively.

### 3. Review in same commit enforced
This review is in the same commit as the STATUS closing entry.

---

## Exit checklist
- [x] Build clean on Ubuntu and macOS
- [x] 666 tests pass (545 unchanged + 121 new)
- [x] Camera Trackball + Orbit functional
- [x] Phong shader compiles and renders
- [x] Scatter3D with spatial grid hit-test
- [x] Surface3D mesh generation + normals
- [x] VolumeItem with transfer function
- [x] Streamlines RK4 integration
- [x] Marching Cubes isosurface extraction
- [x] PBR shader with material system
- [x] ADR-043-048 committed
- [x] This review in SAME commit as STATUS close
- [x] vphase-8 tag (this commit)
