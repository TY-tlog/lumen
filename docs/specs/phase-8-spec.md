# Phase 8 — Modern 3D Engine

## Goal

Add a modern, GPU-accelerated 3D rendering engine alongside the
existing 2D plot stack. Five new 3D plot types (Scatter3D,
Surface3D, Volume rendering, Streamlines, Isosurfaces) bring
Lumen into territory that MATLAB figure windows handle poorly
or not at all. Crucially, every 3D element is editable through
the same double-click paradigm established in Phase 3a/3b for
2D — preserving the MATLAB figure soul while modernizing the
rendering pipeline. Two camera modes (Trackball / Orbit) and
two lighting pipelines (Phong / PBR) give users explicit
control over rendering style — extending the originality
pattern of Phase 7's selectable reactive modes.

This is the largest single phase in the project. It introduces
an entirely new rendering pipeline, a new widget hierarchy
(PlotCanvas3D), 3D hit-testing via ray casting, and a material
system. It builds directly on Phase 7's GPU layer infrastructure
(ADR-039) which was explicitly designed to generalize to 3D.

## Design decisions (approved)

1. **3D backend**: OpenGL 4.5 directly, on top of Phase 7's
   QOpenGLWidget GPU layer
2. **Camera**: user-selectable Trackball / Orbit per plot
   (originality pattern, mirrors Phase 7's reactive mode
   selection)
3. **Plot type scope**: full — Scatter3D, Surface3D, Volume
   rendering (ray marching), Streamlines, Isosurfaces
4. **Lighting**: dual pipeline — Phong default + user-selectable
   PBR with material properties
5. **Coexistence with 2D**: separate PlotCanvas3D widget; 2D and
   3D PlotItems live in distinct hierarchies
6. **Editing**: same double-click paradigm as Phase 3a/3b;
   hit-test via ray casting

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: six internal sub-phases

Phase 8 is delivered as six sub-phases with explicit gates.
Each sub-phase ends with human verification before the next
starts.

- **8.1** 3D rendering foundation (PlotCanvas3D, OpenGL 4.5
  context, Camera system, Phong lighting, Scene3D)
- **8.2** Scatter3D (first PlotItem3D, instanced rendering,
  double-click editing)
- **8.3** Surface3D (Grid2D → mesh, normal computation, Phong
  shading)
- **8.4** Volume rendering (3D texture, ray marching shader,
  transfer function editor)
- **8.5** Streamlines + Isosurfaces (vector field RK4
  integration, Marching Cubes)
- **8.6** PBR lighting pipeline (material system, IBL, light
  editor)

## Phase 8.1 — 3D Foundation

### Technical deliverables

#### plot3d/PlotCanvas3D.{h,cpp}

Separate widget from PlotCanvas (2D). Hosts QOpenGLWidget
directly (not as overlay child — this is a 3D-native canvas).
OpenGL 4.5 core profile context.
class PlotCanvas3D : public QOpenGLWidget,
protected QOpenGLFunctions_4_5_Core {
Q_OBJECT
public:
explicit PlotCanvas3D(QWidget* parent = nullptr);
void addItem(std::unique_ptr<PlotItem3D> item);
Scene3D* scene();
Camera* camera();
void setCameraMode(CameraMode mode);
protected:
void initializeGL() override;
void paintGL() override;
void resizeGL(int w, int h) override;
void mousePressEvent(QMouseEvent*) override;
void mouseMoveEvent(QMouseEvent*) override;
void wheelEvent(QWheelEvent*) override;
void mouseDoubleClickEvent(QMouseEvent*) override;
private:
std::unique_ptr<Scene3D> m_scene;
std::unique_ptr<Camera> m_camera;
CameraMode m_cameraMode{CameraMode::Trackball};
Renderer3D* m_renderer{nullptr};
};

#### plot3d/Scene3D.{h,cpp}

Holds vector of PlotItem3D, Light vector, render bounds.

#### plot3d/Camera.{h,cpp}
enum class CameraMode { Trackball, Orbit };
class Camera {
public:
QMatrix4x4 viewMatrix() const;
QMatrix4x4 projectionMatrix(float aspect) const;
void setMode(CameraMode);
CameraMode mode() const;

// Mode-specific interaction handlers
void handleDrag(QPointF delta);     // Trackball: arcball rotation
                                    // Orbit: yaw/pitch around target
void handleWheel(double delta);     // both: zoom
void handlePan(QPointF delta);      // both: translate target

// State
QVector3D position() const;
QVector3D target() const;
QVector3D up() const;
void setPosition(QVector3D);
void setTarget(QVector3D);

// Persisted in workspace
QJsonObject toJson() const;
void fromJson(const QJsonObject&);
};

Trackball: arcball rotation, free 6-DOF orientation.
Orbit: constrained to azimuth/elevation around fixed target.

#### plot3d/Light.{h,cpp}
enum class LightType { Directional, Point, Ambient };
struct Light {
LightType type;
QVector3D position;     // for Point
QVector3D direction;    // for Directional
QVector3D color;        // RGB intensity
float intensity;
};

Default scene: 1 directional (key) + 1 ambient.

#### plot3d/Renderer3D.{h,cpp}
class Renderer3D {
public:
void initialize();  // compile shaders, set GL state
void render(Scene3D& scene, Camera& camera, QSize viewport);
private:
void renderItem(PlotItem3D& item, const QMatrix4x4& vp,
const std::vector<Light>& lights);
ShaderProgram m_phongProgram;  // Phase 8.1 default
// PBR program added in Phase 8.6
};

#### plot3d/PlotItem3D.{h,cpp}

Abstract base, parallel to 2D PlotItem (does NOT inherit from
PlotItem — separate hierarchies per design decision 5).
class PlotItem3D {
public:
enum class Type {
Scatter3D, Surface3D, Volume, Streamlines, Isosurface
};
virtual ~PlotItem3D() = default;
virtual Type type() const = 0;
virtual QString name() const = 0;
virtual BoundingBox3D dataBounds() const = 0;

// Render: called by Renderer3D with current shader bound
virtual void render(ShaderProgram& shader,
                    const RenderContext& ctx) = 0;

// Hit-test: ray casting
virtual std::optional<HitResult3D> hitTestRay(
    const Ray& ray, double maxDistance) const = 0;

virtual bool isVisible() const = 0;
};

#### plot3d/Ray.{h,cpp}

Ray casting from screen pixel through camera unprojection.
struct Ray {
QVector3D origin;
QVector3D direction;  // normalized
static Ray fromScreenPixel(QPoint pixel,
                           const Camera& cam,
                           QSize viewport);
};

#### Phong shader

GLSL 4.5 vertex + fragment. Per-vertex normal, per-fragment
lighting. One directional light + ambient + per-material
diffuse/specular coefficients.

### Tests 8.1

- test_camera_trackball.cpp: arcball rotation, drag delta → orientation
- test_camera_orbit.cpp: azimuth/elevation constraint
- test_camera_persistence.cpp: JSON roundtrip
- test_ray_unprojection.cpp: known pixel → known ray for fixed camera
- test_scene3d.cpp: add/remove PlotItem3D, bounds aggregation
- test_plotcanvas3d_init.cpp: GL context creation, shader compilation
- test_phong_shader.cpp: render unit cube to FBO, verify lighting
  (compare central pixel color to expected within tolerance)

Gate M8.1: build clean, GL context creates on Ubuntu and macOS,
all 545 Phase 7 tests still pass unchanged.

## Phase 8.2 — Scatter3D

### plot3d/Scatter3D.{h,cpp}
class Scatter3D : public PlotItem3D {
public:
Scatter3D(Dataset* xData, Dataset* yData, Dataset* zData,
Dataset* colorData = nullptr);
void setMarkerShape(MarkerShape3D);  // Sphere, Cube, Tetra
void setMarkerSize(float worldUnits);
void setColor(QColor uniform);
void setColormap(Colormap, MinMax range);  // when colorData != null

Type type() const override { return Type::Scatter3D; }
void render(ShaderProgram&, const RenderContext&) override;
std::optional<HitResult3D> hitTestRay(const Ray&, double) const override;
BoundingBox3D dataBounds() const override;
};

GPU instanced rendering: one VBO with marker geometry (e.g.
icosphere for Sphere), per-instance attributes (position, color)
streamed from Dataset. Handles 1M+ points at 60 FPS target.

### Hit-test

Ray-sphere intersection per marker. For 1M+ markers, accelerated
by spatial grid (uniform grid in world space, cell size = 4×
marker radius). Find closest marker within maxDistance.

### Double-click → Scatter3DPropertyDialog

Same paradigm as Phase 3a LinePropertyDialog. Bundled command
(ChangeScatter3DPropertiesCommand) for undo.

### Tests 8.2

- test_scatter3d_render.cpp: 100 points to FBO, verify pixel
  centers
- test_scatter3d_hit_ray.cpp: known ray → known hit point
- test_scatter3d_spatial_grid.cpp: 100k points, hit accuracy +
  performance
- test_scatter3d_dialog.cpp: dialog opens on double-click,
  property changes via CommandBus
- test_scatter3d_workspace.cpp: save/load roundtrip with all
  properties

Gate M8.2: human verifies — open synthetic Volume Sphere sample,
add Scatter3D from a TabularBundle with x/y/z columns, rotate,
zoom, double-click marker, edit color.

## Phase 8.3 — Surface3D

### plot3d/Surface3D.{h,cpp}

Renders Grid2D as a height-mapped or flat-colored surface mesh.
class Surface3D : public PlotItem3D {
public:
Surface3D(Grid2D* grid);
enum class Mode { HeightMap, FlatColored, Both };
void setMode(Mode);
void setColormap(Colormap);
void setHeightScale(float);
void setWireframe(bool);
void setOpacity(float);
// ...
};

Mesh generation: triangulate Nx×Ny grid into 2(Nx-1)(Ny-1)
triangles. Vertex normals via averaged face normals. Recompute
on Grid2D::changed() (reactive).

### Tests 8.3

- test_surface3d_mesh.cpp: known Grid2D → expected vertex count
  + topology
- test_surface3d_normals.cpp: smooth sphere grid → normals match
  analytic
- test_surface3d_reactive.cpp: Grid2D mutation → mesh refit (DAG mode)
- test_surface3d_workspace.cpp: roundtrip
- test_surface3d_hit_ray.cpp: ray-triangle intersection

Gate M8.3: human verifies — open Gaussian 2D sample as Surface3D
in PlotCanvas3D, switch height/flat/both modes, change colormap.

## Phase 8.4 — Volume Rendering

### plot3d/VolumeItem.{h,cpp}
class VolumeItem : public PlotItem3D {
public:
VolumeItem(Volume3D* volume);
void setTransferFunction(TransferFunction);
void setSampleStep(float);  // ray march step size
void setIsoMode(bool);       // iso-surface accumulation
void setMaxSamples(int);     // performance cap
};

### plot3d/TransferFunction.{h,cpp}
class TransferFunction {
public:
void addControlPoint(double scalarValue, QColor color, double opacity);
void removeControlPoint(int index);
// Evaluated to a 1D RGBA texture (256 or 1024 samples) for
// GPU sampling
QImage toLUT(int resolution) const;

QJsonObject toJson() const;
};

### Ray marching shader

GLSL fragment shader: for each fragment, compute entry/exit of
volume bounding box, march from entry to exit at sampleStep,
accumulate front-to-back compositing. 3D texture (GL_R32F or
GL_R16F) for volume data, 1D texture for transfer function LUT.

### ui/TransferFunctionEditor.{h,cpp}

Visual editor: histogram of volume scalar values + draggable
control points + color picker per point. Live preview in
VolumeItem.

### Tests 8.4

- test_transfer_function.cpp: control points → LUT correctness
- test_volume_render_synthetic.cpp: synthetic volume (e.g. sphere)
  renders expected silhouette
- test_volume_sample_step.cpp: smaller step → smoother result
- test_transfer_function_editor.cpp: GUI interactions
- test_volume_workspace.cpp: roundtrip including TransferFunction

Gate M8.4: human verifies — load Volume Sphere sample as
VolumeItem, edit transfer function, observe live update.

## Phase 8.5 — Streamlines + Isosurfaces

### plot3d/Streamlines.{h,cpp}

Vector field on Grid2D × component count = 3 (vx, vy, vz at
each point), or three Grid2D datasets. Integrate streamlines
via 4th-order Runge-Kutta from seed points.
class Streamlines : public PlotItem3D {
public:
Streamlines(Volume3D* vx, Volume3D* vy, Volume3D* vz);
void setSeedPoints(std::vector<QVector3D>);
void setSeedGrid(int resolution);  // auto-generate seeds
void setIntegrationStep(float);
void setMaxSteps(int);
void setColorByMagnitude(bool);
void setColormap(Colormap);
};

Render as line strips (thin tubes optional via geometry shader,
deferred to Phase 9 if expensive).

### plot3d/Isosurface.{h,cpp}

Marching Cubes from Volume3D at user-specified iso-value(s).
class Isosurface : public PlotItem3D {
public:
Isosurface(Volume3D* volume);
void setIsoValues(std::vector<double>);
void setColors(std::vector<QColor>);  // one per iso value
void setOpacity(float);
};

Mesh generated CPU-side via Marching Cubes (Lorensen & Cline
1987). Recompute on Volume3D::changed() in DAG mode (potentially
slow — guard with debounce).

### Tests 8.5

- test_streamlines_rk4.cpp: known vector field (e.g. circular)
  → known streamline shape
- test_streamlines_seed_grid.cpp: auto seed generation
- test_marching_cubes.cpp: synthetic sphere volume → mesh with
  expected vertex count, surface area within tolerance
- test_isosurface_multiple.cpp: 3 iso values → 3 nested meshes
- test_isosurface_reactive.cpp: Volume3D mutation → mesh refit

Gate M8.5: human verifies — synthetic curl field renders
streamlines; sphere volume renders isosurface at half-max.

## Phase 8.6 — PBR Lighting

### Material system
struct PbrMaterial {
QColor baseColor;
float metallic;      // 0..1
float roughness;     // 0..1
float ior;           // index of refraction
QColor emissive;
};

PlotItem3D types gain optional PbrMaterial. If unset, Phong is
used (default for backward compatibility with Phase 8.1-8.5).

### PBR shader

GLSL: Cook-Torrance BRDF, GGX distribution, Smith geometry,
Fresnel-Schlick. Multi-light support. Optional IBL via cubemap
texture (default white-furnace cubemap if none provided).

### ui/LightingDialog.{h,cpp}

Per-PlotItem3D dialog with:
- Lighting model toggle: Phong / PBR
- If PBR: material editor (color, metallic slider, roughness
  slider, IOR, emissive)
- Scene-level: light editor (add/remove/edit lights, IBL cubemap
  load)

### Tests 8.6

- test_pbr_shader.cpp: render PBR sphere, compare to reference
  image
- test_phong_pbr_switch.cpp: same scene rendered with both,
  visible difference
- test_material_workspace.cpp: roundtrip
- test_lighting_dialog.cpp: GUI toggles work, CommandBus
  recorded
- test_ibl_cubemap.cpp: cubemap load + sampling

Gate M8.6: human verifies — render Surface3D with both Phong
and PBR; toggle metallic on Surface3D, observe specular
highlight change.

## ADRs

- ADR-043 OpenGL 4.5 backend on Phase 7 GPU layer. Alternatives:
  Qt3D (rejected — deprecation risk), Vulkan via QRhi (rejected
  — complexity not justified now), platform-native Metal/Vulkan
  split (rejected — maintenance burden)
- ADR-044 Camera mode selection (Trackball + Orbit). Alternatives:
  Trackball only (rejected — Orbit better for CAD/scientific
  navigation), Orbit only (rejected — Trackball better for
  inspection), all three including First-person (rejected —
  not scientific use case)
- ADR-045 PlotItem3D as separate hierarchy from 2D PlotItem.
  Alternatives: unified hierarchy (rejected — forces every PlotItem
  to handle 3D context, breaks Phase 5 design), bridge pattern
  (rejected — over-abstraction)
- ADR-046 Volume rendering via fragment shader ray marching
  with transfer function LUT. Alternatives: CPU ray marching
  (rejected — too slow), GPU compute shader (rejected — minimum
  GL version + complexity), texture splatting (rejected —
  artifacts on dense volumes)
- ADR-047 Dual lighting pipeline (Phong default + PBR opt-in).
  Alternatives: Phong only (rejected — PBR is modern expectation),
  PBR only (rejected — overkill for many scientific plots),
  unified PBR with Phong as a fixed material preset (considered;
  rejected for clearer user intent)
- ADR-048 Hit-test via screen-space ray casting with spatial
  acceleration. Alternatives: GPU picking via FBO color ID
  (rejected — round-trip latency, doesn't scale to volumes),
  CPU iteration without acceleration (rejected — O(N) per click
  unacceptable above 10k items)

Each ADR lists at least two alternatives.

## Architecture updates

docs/architecture.md "Phase 8 additions" section:

- New plot3d/ submodule (parallel to plot/)
  - PlotCanvas3D, Scene3D, Camera, Light, Renderer3D
  - PlotItem3D abstract base
  - Concrete: Scatter3D, Surface3D, VolumeItem, Streamlines,
    Isosurface
  - Ray, BoundingBox3D, RenderContext
- New plot3d/shaders/ for GLSL files (phong.vert/frag,
  pbr.vert/frag, volume.vert/frag, etc.)
- ui/ additions: ScatterPropertyDialog3D, SurfacePropertyDialog,
  VolumePropertyDialog (with TransferFunctionEditor),
  StreamlinesPropertyDialog, IsosurfacePropertyDialog,
  LightingDialog
- Workspace file format extended: plot3d items serialize
  Camera state, lighting, materials, transfer functions
- Layering: plot3d/ depends on data/, plot/Colormap,
  core/. ui/ depends on plot3d/. plot/ unchanged.

## Acceptance criteria

Manual flow per sub-phase as listed in M-gates above.

Regression at every M-gate:
- [ ] All 545 Phase 7 tests pass unchanged
- [ ] Phase 2-7 functionality unchanged in 2D PlotCanvas
- [ ] Phase 4 workspace saves/loads (v1 + v2 formats)
- [ ] Phase 4 export (PNG/SVG/PDF) for 2D unchanged
- [ ] CI green on Ubuntu Debug/Release + macOS Debug/Release

## Real-data exit criterion

Human loads at least one real 3D scientific dataset (e.g.
microscopy z-stack via TIFF stack loader, simulation volume
via HDF5, point cloud via CSV) and:

- [ ] Visualizes appropriately (Scatter3D for points, Volume
      for z-stacks, Surface3D for height maps, Isosurface for
      structural data, Streamlines for vector fields)
- [ ] Switches camera mode (Trackball ↔ Orbit) as needed
- [ ] Edits at least one element via double-click
- [ ] Toggles PBR lighting on at least one item
- [ ] Saves workspace, reopens, all 3D state restored
- [ ] Exports a 2D snapshot (PDF) of the 3D view for publication

## Non-goals

- 4D / time-varying volumes (later phase)
- Mesh import (STL, OBJ, PLY) — Phase 9 or later
- VR / stereoscopic rendering — not planned
- Hardware ray tracing (RTX) — not planned
- Real-time particle systems beyond Streamlines
- Custom GLSL shader injection by user — Phase 16 (plugins)
- Interactive cutting planes / clipping — later phase
- Parametric surface (mathematical surface from formula) —
  later phase

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| OpenGL 4.5 not available on some macOS versions | Detect at init; fall back to 4.1 with feature flag; document |
| Volume rendering performance on large volumes | sampleStep adaptive; maxSamples cap; status bar FPS hint |
| Marching Cubes mesh size for high-res volumes | Threshold + warning above 10M triangles |
| PBR shader complexity slows Scatter3D with 1M points | PBR materials opt-in per item; default Phong fast |
| Trackball arcball math edge cases (poles) | Quaternion-based; test fixtures for pole crossings |
| Spatial grid memory for 10M point Scatter3D | Configurable cell size; document budget |
| Workspace file size grows with volume + transfer function | Volumes referenced by path, not embedded; transfer functions inlined (small) |
| GL context creation in headless CI | offscreen platform + EGL fallback as in Phase 7 |
| Reactive volume updates trigger expensive Marching Cubes | Debounce 200 ms; Phase 7 mode-aware |

## Task breakdown

### Architect (S)
- phase-8-plan.md with 6 sub-phase structure and 6 M-gates
- ADR-043 through ADR-048
- Architecture update
- STATUS opening entry

### Backend (XL, across sub-phases)
- 8.1: Camera, Light, Scene3D, Renderer3D, PlotItem3D base,
  Ray, Phong shader
- 8.2: Scatter3D + spatial grid + commands
- 8.3: Surface3D + mesh + normals
- 8.4: VolumeItem + ray marching shader + TransferFunction
- 8.5: Streamlines + RK4 + Marching Cubes + Isosurface
- 8.6: PBR shader + material system + IBL

### Frontend (XL)
- 8.1: PlotCanvas3D widget + camera mode UI
- 8.2: Scatter3DPropertyDialog
- 8.3: SurfacePropertyDialog
- 8.4: VolumePropertyDialog + TransferFunctionEditor
- 8.5: StreamlinesPropertyDialog + IsosurfacePropertyDialog
- 8.6: LightingDialog (Phong/PBR toggle + material editor +
  light editor + IBL load)

### QA (L)
- Per-sub-phase tests as listed
- Regression gate at each M point: 545 tests unchanged
- CI green on 4 platforms

### Integration (S)
- Sub-phase merge windows
- vphase-8 tag at exit

### Docs (S)
- README update (3D capability)
- plot3d/CLAUDE.md (new submodule)
- phase-8-review.md WRITTEN AND COMMITTED IN THE SAME COMMIT
  AS THE CLOSING .lumen-ops/STATUS.md ENTRY. This rule appears
  verbatim in the closing task's description, not only in exit
  checklist. Phase 3b/4/5/6/7 proved this discipline; Phase 8
  follows it.

## Exit checklist

Phase 8.1 — 3D foundation:
- [ ] OpenGL 4.5 context creates on Ubuntu and macOS
- [ ] Camera Trackball and Orbit both functional
- [ ] Phong shader compiles and renders unit cube
- [ ] All 545 Phase 7 tests pass unchanged

Phase 8.2 — Scatter3D:
- [ ] Scatter3D renders 1M points at >= 30 FPS
- [ ] Hit-test returns correct marker within 100 ms for 1M points
- [ ] Double-click opens Scatter3DPropertyDialog
- [ ] Workspace roundtrip preserves all properties

Phase 8.3 — Surface3D:
- [ ] Grid2D mesh generation with correct topology
- [ ] Vertex normals smooth (verified on sphere grid)
- [ ] Reactive update on Grid2D changed() in DAG mode
- [ ] Wireframe + flat + height modes all work

Phase 8.4 — Volume:
- [ ] VolumeItem renders synthetic sphere correctly
- [ ] TransferFunctionEditor live preview
- [ ] sampleStep affects quality/speed as expected

Phase 8.5 — Streamlines + Isosurfaces:
- [ ] Streamlines RK4 integrates correctly on known vector field
- [ ] Marching Cubes produces topologically consistent mesh
- [ ] Multiple iso values render distinct nested surfaces

Phase 8.6 — PBR:
- [ ] PBR shader matches reference image on standard sphere
- [ ] Phong/PBR toggle per PlotItem3D works
- [ ] Material editor changes reflected immediately
- [ ] IBL cubemap load functional

Phase 8 overall:
- [ ] Build clean on Ubuntu and macOS, Debug + Release
- [ ] All 545 Phase 7 tests pass unchanged
- [ ] New Phase 8 tests pass (target 545 → 700+)
- [ ] CI green on all 4 platforms
- [ ] Real-data exit criterion passed
- [ ] ADR-043 through ADR-048 committed
- [ ] phase-8-review.md committed IN SAME COMMIT as closing
      STATUS entry
- [ ] vphase-8 tag pushed
