# Phase 8 Plan — Modern 3D Engine

> Reference: `docs/specs/phase-8-spec.md`

## Load-bearing decisions

### Decision 1: PlotItem3D fully separate from PlotItem
No shared base class. Parallel hierarchies. CommandBus uses
variant<PlotItem*, PlotItem3D*> in commands. HitTester dispatch
is per-canvas (2D or 3D). WorkspaceFile uses "plot" vs "plot3d"
discriminator blocks. See ADR-045.

### Decision 2: Reactive — per-item binding + debounced mesh + lazy volume snapshot
Each PlotItem3D gets a ReactiveBinding. Mesh-heavy items
(Surface3D, Isosurface) debounce regeneration to 200ms. Static
mode on Volume3D uses lazy snapshot (reference + generation
stamp) instead of deep copy. See plan T2 and ADR-038 extension.

### Decision 3: Parallelism
```
8.1 Foundation ──┬──→ 8.2 Scatter3D
                 ├──→ 8.3 Surface3D
                 ├──→ 8.4 Volume
                 └──→ 8.6 PBR
                       8.3 + 8.4 ──→ 8.5 Streamlines+Iso
```

## Hard rules

1. **Review-in-same-commit** (T27 verbatim).
2. **545-test regression** at every M-gate.
3. **CI green on 4 platforms** at every merge.
4. **GL context in CI**: QT_QPA_PLATFORM=offscreen + Mesa llvmpipe.

---

## Sub-phase 8.1 — 3D Foundation (BLOCKS ALL)

### T1 — Camera + CameraMode (Backend, Size: M)

**Files**: `src/lumen/plot3d/Camera.h/.cpp`

Trackball (arcball quaternion rotation) + Orbit (azimuth/elevation).
JSON persistence. viewMatrix(), projectionMatrix(aspect).

Tests: test_camera_trackball, test_camera_orbit, test_camera_json.

**Dependencies**: none (day 1)

### T2 — Light + Scene3D (Backend, Size: S)

**Files**: `src/lumen/plot3d/Light.h`, `src/lumen/plot3d/Scene3D.h/.cpp`

Light struct (Directional/Point/Ambient). Scene3D: vector<PlotItem3D>,
vector<Light>, BoundingBox3D aggregation.

Tests: test_scene3d.

### T3 — PlotItem3D + BoundingBox3D + Ray (Backend, Size: M)

**Files**: `src/lumen/plot3d/PlotItem3D.h`, `src/lumen/plot3d/BoundingBox3D.h`,
`src/lumen/plot3d/Ray.h/.cpp`, `src/lumen/plot3d/RenderContext.h`

Abstract base with Type enum, virtual render/hitTestRay/dataBounds.
Ray::fromScreenPixel for 3D picking.

Tests: test_ray_unprojection.

### T4 — Renderer3D + Phong shader (Backend, Size: L)

**Files**: `src/lumen/plot3d/Renderer3D.h/.cpp`,
`src/lumen/plot3d/ShaderProgram.h/.cpp`,
`src/lumen/plot3d/shaders/phong.vert`, `src/lumen/plot3d/shaders/phong.frag`

OpenGL 4.5 core profile. Shader compilation, GL state management.
Per-item render with current shader + lights.

Tests: test_phong_shader (render cube to FBO, check pixel color).

### T5 — PlotCanvas3D widget (Frontend, Size: L)

**Files**: `src/lumen/ui/PlotCanvas3D.h/.cpp`

QOpenGLWidget + QOpenGLFunctions_4_5_Core. initializeGL/paintGL/resizeGL.
Mouse events: left-drag camera, wheel zoom, double-click hit-test.
Camera mode toggle widget.

Tests: test_plotcanvas3d_init.

### T6 — plot3d CMake + CI (Backend, Size: S)

**Files**: `src/lumen/plot3d/CMakeLists.txt`

Static library lumen_plot3d. Link Qt6::OpenGLWidgets, Qt6::Gui.
Shader files embedded via qt_add_resources.
CI: verify Mesa llvmpipe provides GL 4.5 in offscreen mode.

### T7 — QA 8.1 Tests (QA, Size: M)

7 test files per spec. ≥20 tests. 545 unchanged.

---

### M8.1 — Gate (BLOCKS 8.2-8.6)

GL context on Ubuntu+macOS. Phong renders cube. Camera works.
545 Phase 7 tests unchanged. CI green.

---

## Sub-phase 8.2 — Scatter3D (parallel after M8.1)

### T8 — Scatter3D + spatial grid (Backend, Size: L)

**Files**: `src/lumen/plot3d/Scatter3D.h/.cpp`,
`src/lumen/plot3d/SpatialGrid3D.h/.cpp`

Instanced rendering: icosphere VBO + per-instance position/color.
SpatialGrid3D for ray-cast acceleration (uniform grid, cell = 4×radius).

Tests: render, hit-ray, spatial grid perf.

### T9 — ChangeScatter3DPropertiesCommand (Backend, Size: S)

Bundled command (markerShape, markerSize, color, colormap, name, visible).

### T10 — Scatter3DPropertyDialog (Frontend, Size: M)

Marker shape combo (Sphere/Cube/Tetra), size, color, colormap picker.

### T11 — QA 8.2 Tests (QA, Size: M)

5 tests per spec. Workspace roundtrip.

---

### M8.2 — Gate

1M points ≥ 30 FPS. Hit-test < 100ms. Double-click edit works.

---

## Sub-phase 8.3 — Surface3D (parallel after M8.1)

### T12 — Surface3D mesh + normals (Backend, Size: L)

**Files**: `src/lumen/plot3d/Surface3D.h/.cpp`

Grid2D → triangle mesh. Vertex normals via face-normal averaging.
HeightMap/FlatColored/Both modes. Wireframe overlay.
Reactive: Grid2D::changed() ��� remesh (debounced 200ms).

Tests: mesh topology, normals, reactive remesh.

### T13 — SurfacePropertyDialog + command (Frontend, Size: M)

Mode combo, colormap, height scale, wireframe toggle, opacity.

### T14 — QA 8.3 Tests (QA, Size: S)

5 tests + workspace roundtrip.

---

### M8.3 — Gate

Gaussian 2D renders correctly. All modes work.

---

## Sub-phase 8.4 — Volume Rendering (parallel after M8.1)

### T15 — VolumeItem + ray marching shader (Backend, Size: XL)

**Files**: `src/lumen/plot3d/VolumeItem.h/.cpp`,
`src/lumen/plot3d/shaders/volume.vert/.frag`

Volume3D → GL_R32F 3D texture. Fragment shader: front-to-back
compositing with transfer function LUT (1D RGBA texture).
Adaptive sample step. maxSamples performance cap.

### T16 — TransferFunction + editor (Backend+Frontend, Size: L)

**Files**: `src/lumen/plot3d/TransferFunction.h/.cpp`,
`src/lumen/ui/TransferFunctionEditor.h/.cpp`

Control points (value → color + opacity). toLUT(resolution).
Visual editor: histogram + draggable points + color picker.

### T17 — ChangeVolumePropertiesCommand (Backend, Size: S)

Bundled: transfer function, sample step, max samples, name, visible.

### T18 — QA 8.4 Tests (QA, Size: M)

5 tests: transfer function, synthetic render, sample step, editor, workspace.

---

### M8.4 — Gate

Synthetic sphere renders. Transfer function live preview works.

---

## Sub-phase 8.5 — Streamlines + Isosurfaces (after M8.3 + M8.4)

### T19 — Streamlines + RK4 (Backend, Size: L)

**Files**: `src/lumen/plot3d/Streamlines.h/.cpp`

RK4 integration from seed points. Line strip rendering.
Color by magnitude with colormap. Auto-seed grid.

### T20 — Isosurface + Marching Cubes (Backend, Size: L)

**Files**: `src/lumen/plot3d/Isosurface.h/.cpp`,
`src/lumen/plot3d/MarchingCubes.h/.cpp`

Lorensen & Cline 1987. 256-entry edge table + tri table.
Multiple iso values → multiple meshes. Reactive with debounce.

### T21 — Dialogs + commands (Frontend, Size: M)

StreamlinesPropertyDialog, IsosurfacePropertyDialog.
ChangeStreamlinesCommand, ChangeIsosurfaceCommand.

### T22 — QA 8.5 Tests (QA, Size: M)

5 tests: RK4, seed grid, marching cubes, multiple iso, reactive.

---

### M8.5 — Gate

Known vector field streamlines correct. Sphere iso-surface consistent.

---

## Sub-phase 8.6 — PBR Lighting (parallel after M8.1)

### T23 — PBR shader (Backend, Size: L)

**Files**: `src/lumen/plot3d/shaders/pbr.vert/.frag`,
`src/lumen/plot3d/PbrMaterial.h`

Cook-Torrance BRDF, GGX, Smith geometry, Fresnel-Schlick.
Multi-light. Optional IBL cubemap.

### T24 — Material system + IBL (Backend, Size: M)

PbrMaterial struct. Per-PlotItem3D optional attachment.
IBL cubemap loading + default white-furnace.

### T25 — LightingDialog (Frontend, Size: M)

Phong/PBR toggle per item. Material editor (baseColor, metallic,
roughness, IOR, emissive sliders). Light editor. IBL load.

### T26 — QA 8.6 Tests (QA, Size: M)

5 tests: PBR vs Phong, material workspace, IBL, dialog, toggle.

---

### M8.6 — Gate

PBR vs Phong toggle works. Material editor live.

---

## Closing

### T27 — Docs Closing (REVIEW AND STATUS IN SAME COMMIT)

**HARD RULE (verbatim, Phase 3b/4/5/6/7 lesson)**: docs/reviews/
phase-8-review.md MUST be WRITTEN AND COMMITTED IN THE SAME COMMIT
as the closing .lumen-ops/STATUS.md entry. One commit, one `git add`,
one `git commit`. The coordinator MUST enforce this.

**Files in ONE commit**:
- `docs/reviews/phase-8-review.md`
- `.lumen-ops/STATUS.md`
- `README.md` (3D capability)
- `src/lumen/plot3d/CLAUDE.md` (new submodule)

After: `git tag vphase-8 && git push origin main --tags`.

---

## Parallel Execution Schedule

```
Round 1 (8.1, ~8h):
  Backend: T1-T4, T6 (Camera, Light, Scene, Renderer, Phong, CMake)
  Frontend: T5 (PlotCanvas3D)
  QA: T7

M8.1 GATE

Round 2 (8.2+8.3+8.4+8.6 parallel, ~12h):
  Backend: T8+T9 (Scatter3D) | T12 (Surface3D) | T15-T17 (Volume) | T23-T24 (PBR)
  Frontend: T10 (Scatter3DDialog) | T13 (SurfaceDialog) | T16 (TF Editor) | T25 (LightingDialog)
  QA: T11 | T14 | T18 | T26

M8.2 + M8.3 + M8.4 + M8.6 GATES

Round 3 (8.5, ~6h):
  Backend: T19 (Streamlines) + T20 (Isosurface)
  Frontend: T21 (Dialogs)
  QA: T22

M8.5 GATE

Round 4 (closing, ~30min):
  T27 (review+STATUS SAME commit, tag)
```

**Total: 4 sequential rounds, ~26-30 hours.**

## Risks

- GL 4.5 not on all macOS → fallback to 4.1 with feature flags
- Volume rendering perf → adaptive sample step + maxSamples
- Marching Cubes mesh explosion → threshold + warning
- Headless CI GL → Mesa llvmpipe + EGL
- Static mode on Volume3D → lazy snapshot (Decision 2)
