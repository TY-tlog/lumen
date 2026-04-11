# Phase 7 Plan — Reactive Plot Engine + 2D Scalar Fields

> Reference: `docs/specs/phase-7-spec.md`

## Hard rules

1. **Review in same commit**: phase-7-review.md and closing
   STATUS entry committed together. Verbatim in T23.
2. **410-test regression gate** at each sub-phase merge.
3. **Static mode = snapshot on bind** (see ADR-038). Deep-copy
   on entering Static. MemoryManager tracks snapshot allocation.
4. **PlotCanvas hosts GPU layer** (see ADR-039). QOpenGLWidget
   child, lazy creation, managed by PlotCanvas.
5. **Perceptual uniformity**: CIELAB ΔE variance metric with
   explicit threshold (ADR-040).
6. **CONREC from scratch**: Paul Bourke 1987 reference (ADR-041).

---

## Sub-phase 7.1 — Reactive Core

### T1 — DependencyGraph + ReactiveBinding + ReactiveMode (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/core/reactive/ReactiveMode.h`
- `src/lumen/core/reactive/DependencyGraph.h` / `.cpp`
- `src/lumen/core/reactive/ReactiveBinding.h` / `.cpp`
- `src/lumen/core/reactive/CMakeLists.txt`
- `tests/unit/test_reactive_mode.cpp`
- `tests/unit/test_dependency_graph.cpp`

**ReactiveMode enum**: Static, DAG, Bidirectional.

**DependencyGraph**: QObject. `addNode(Dataset*)`,
`addDerivation(Dataset* src, Dataset* derived, std::function)`,
`propagate(Dataset* changed)` — cascades updates through DAG.
`downstream(Dataset*)` returns transitive dependents.
Generation counter: incremented on each propagate call.
Cycle detection: reject addDerivation that would create a cycle.

**ReactiveBinding**: QObject. Per-plot binding.
- Constructor: `ReactiveBinding(PlotItem* plot, Mode mode)`
- `setMode(Mode m)`:
  - **Static**: disconnect from Dataset::changed(). Deep-copy
    the bound Dataset into `snapshot_` (unique_ptr<Dataset>).
    Register snapshot with MemoryManager. PlotItem reads from
    snapshot via `dataSource()` accessor.
  - **DAG**: destroy snapshot (dealloc from MemoryManager).
    Connect Dataset::changed() → plot->invalidate() via
    DependencyGraph. PlotItem reads live Dataset.
  - **Bidirectional**: same as DAG + install CommandBus
    interceptor for write-back. Generation counter prevents
    feedback loop.
- `bindDataset(Dataset* ds)`: stores the live dataset ref.
- `Dataset* dataSource() const`: returns snapshot_ in Static,
  live dataset in DAG/Bidirectional.
- `invalidate()`: in Static mode, re-snapshots from live
  dataset and re-registers with MemoryManager.

**Acceptance**: mode transitions work, snapshot isolation verified,
MemoryManager tracks snapshot, generation counter prevents
bidirectional feedback loop. ≥10 tests including:
- Generation counter increments on each propagate call
- Cycle detection rejects circular addDerivation
- Bidirectional write-back does NOT trigger infinite
  re-propagation (generation counter suppresses same-gen signal)
- Static snapshot is NOT affected by live Dataset mutation
- MemoryManager allocation tracked for snapshot

**Dependencies**: none (day 1)

---

### T2 — Integrate ReactiveBinding with Dataset::changed() (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp` — owns ReactiveBinding
  per plot, creates with default mode Static
- `src/lumen/plot/PlotItem.h` — add `invalidate()` virtual method
  (triggers repaint request)

**Acceptance**: DAG mode: modifying a Dataset triggers plot repaint.
Static mode: modifying a Dataset does NOT change the plot.

**Dependencies**: T1

---

### T3 — ReactivityModeWidget (Frontend, Size: S)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/ReactivityModeWidget.h` / `.cpp`

**Widget**: 3-position toggle (Static / DAG / Bidirectional) with
tooltips explaining each. Emits `modeChanged(Mode)`.

Integrated into existing property dialogs (Line, Scatter, Bar)
as an additional section at the bottom: "Reactivity: [Static ○]
[DAG ●] [Bidir ○]".

**Dependencies**: T2

---

### T4 — QA Reactive Tests (QA, Size: M)

**Owner**: qa

6 test files per spec:
- test_reactive_mode.cpp: mode transitions, enum values
- test_dependency_graph.cpp: add, propagate, downstream, cycle
  detection, generation counter
- test_static_mode.cpp: snapshot isolation — mutate live Grid2D,
  verify Static plot's dataSource() still returns original values
- test_dag_mode.cpp: Dataset::changed() triggers plot repaint
  within one event loop cycle
- test_bidirectional_mode.cpp: plot edit writes back to Dataset.
  **MUST include**: bidirectional write-back does NOT trigger
  infinite re-propagation (generation counter test)
- test_mode_switching.cpp: switch mode at runtime without data loss

**Acceptance**: ≥20 tests, 410 existing tests unchanged.

**Dependencies**: T1, T2, T3

---

### M7.1 — Gate

All 3 modes work. Static snapshot isolation verified. 410 Phase 6
tests unchanged. Target: ≥428 total.

---

## Sub-phase 7.2 — Heatmap + Colormap

### T5 — Colormap System (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/Colormap.h` / `.cpp`
- `tests/unit/test_colormap_builtin.cpp`
- `tests/unit/test_colormap_perceptual.cpp`
- `tests/unit/test_colormap_colorblind.cpp`
- `tests/unit/test_colormap_json_roundtrip.cpp`

**Colormap**: stores vector of control points (t, QColor).
`sample(double t)` interpolates in sRGB (or CIELAB for uniformity).

10 built-in maps: Viridis, Plasma, Inferno, Magma, Turbo, Cividis,
Gray, Hot, Cool, RedBlue, BrownTeal. Control points hardcoded from
matplotlib/scientific sources.

`isPerceptuallyUniform()`: sample N=256 points, convert to CIELAB,
compute ΔE between adjacent pairs, check variance < threshold T
(ADR-040 specifies T).

`isColorblindSafe()`: simulate deuteranopia/protanopia/tritanopia
via Machado 2009 matrices, check that ΔE between adjacent simulated
colors > minimum threshold.

JSON I/O: `toJson()` / `fromFile(path)` for user custom colormaps.

**Acceptance**: 10 built-ins sample correctly, viridis/plasma/inferno/
magma/cividis pass uniformity, cividis passes CVD, JSON roundtrip.
≥12 tests.

**Dependencies**: none (day 1)

---

### T6 — Heatmap PlotItem (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/Heatmap.h` / `.cpp`
- `tests/unit/test_heatmap_cpu_path.cpp`

**Heatmap : PlotItem**:
- Holds Grid2D* and Colormap
- type() → Type::Heatmap
- paint() CPU path: loop over grid cells, map value → colormap
  color, draw as QImage, blit to QPainter. Clip to plotArea.
- dataBounds() from Grid2D dimensions
- setColormap, setValueRange (manual min/max), setInterpolation
  (nearest/bilinear), setOpacity
- activeRenderPath(): CPU if grid <= 1024×1024, GPU otherwise

**Acceptance**: 256×256 grid renders correctly, value range
override works, ≥4 tests.

**Dependencies**: T5 (Colormap)

---

### T7 — HeatmapRenderer CPU Path (Backend, Size: S)

**Owner**: backend

Heatmap::paint() implementation for CPU. If the grid has
interpolation=bilinear, use QPainter::setRenderHint(SmoothPixmapTransform).

**Dependencies**: T6

---

### T8 — PlotCanvas GPU Layer Infrastructure (Frontend, Size: L)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/PlotCanvas.h` / `.cpp`

Add lazy QOpenGLWidget creation:
- `QOpenGLWidget* gpuLayer()` — creates on first call
- GPU widget is a child of PlotCanvas, positioned to overlay
  the plot area
- PlotCanvas::paintEvent: render CPU items via PlotRenderer,
  then schedule GPU widget repaint for GPU items
- GPU widget has transparent background so CPU content shows
  through

If OpenGL context creation fails (no driver), log warning and
force all items to CPU path.

**Acceptance**: GPU widget created and positioned correctly,
transparent overlay works, fallback to CPU on GL failure.

**Dependencies**: none (day 1, infrastructure)

---

### T9 — Heatmap GPU Path (Frontend, Size: L)

**Owner**: frontend

**Files to create**:
- `src/lumen/plot/HeatmapGpuRenderer.h` / `.cpp`

Upload Grid2D data as GL_R32F texture. Colormap as 256×1 RGBA
LUT texture. Fragment shader: sample data texture, index into
LUT, output color. Handle coordinate mapping via uniforms
(data range → viewport).

**Acceptance**: 2048×2048 grid renders via GPU, visual match to
CPU path on shared test input. ≥2 tests (skip if no GL context).

**Dependencies**: T6, T8

---

### T10 — HeatmapPropertyDialog (Frontend, Size: M)

**Owner**: frontend

Colormap picker (dropdown with preview swatches, uniformity badge
icon), value range min/max, interpolation toggle, opacity slider.

**Dependencies**: T6

---

### T11 — QA Heatmap/Colormap Tests (QA, Size: M)

8 tests per spec. ≥8 new tests.

**Dependencies**: T5-T10

---

### M7.2 — Gate

CPU and GPU paths both work. 5 built-in colormaps verified uniform.
Cividis CVD-safe. 410 existing tests unchanged. Target: ≥448 total.

---

## Sub-phase 7.3 — Contour

### T12 — ContourAlgorithm (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/ContourAlgorithm.h` / `.cpp`
- `tests/unit/test_contour_algorithm.cpp`
- `tests/unit/test_contour_degenerate.cpp`

CONREC implementation (Paul Bourke 1987): triangular subdivision
of each grid cell, contour line extraction per level. Handles:
saddle points (ambiguous cell), plateau (value exactly on level),
NaN cells (skip). Returns `vector<ContourSegment{QPointF a, b,
double level}>`.

**Acceptance**: gaussian/saddle/monotonic produce expected contours,
degenerate cases handled, ≥6 tests.

**Dependencies**: none (day 1)

---

### T13 — ContourPlot PlotItem (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/ContourPlot.h` / `.cpp`
- `tests/unit/test_contour_auto_levels.cpp`
- `tests/unit/test_contour_with_heatmap.cpp`

**ContourPlot : PlotItem**: holds Grid2D*, levels (manual or auto),
line color, line width, labels visible. paint() calls
ContourAlgorithm::extract, maps segments to pixels, draws lines
and optional labels.

**Acceptance**: auto-levels from data range, overlay on heatmap
renders correctly, ≥4 tests.

**Dependencies**: T12

---

### T14 — ContourPropertyDialog (Frontend, Size: S)

Level count, manual levels editor, line color, width, labels toggle.

**Dependencies**: T13

---

### T15 — QA Contour Tests (QA, Size: S)

4 tests per spec.

**Dependencies**: T12-T14

---

### M7.3 — Gate

Contour overlays heatmap on Gaussian 2D. CONREC handles
degenerate cases. 410 existing tests unchanged. Target: ≥462 total.

---

## Sub-phase 7.4 — Statistical Plots

### T16 — HistogramSeries (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/HistogramSeries.h` / `.cpp`
- `tests/unit/test_histogram.cpp`
- `tests/unit/test_histogram_auto_bins.cpp`

**HistogramSeries : PlotItem**: takes rank-1 Dataset. Computes
bins from data. BinRule enum: Scott, FreedmanDiaconis, Sturges.
Normalization: Count, Density, Probability. paint() draws bars
for each bin. dataBounds includes y=0.

**Acceptance**: 3 binning rules, 3 normalization modes, ≥4 tests.

**Dependencies**: none (day 1)

---

### T17 — BoxPlotSeries (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/BoxPlotSeries.h` / `.cpp`
- `tests/unit/test_boxplot.cpp`
- `tests/unit/test_boxplot_grouped.cpp`

WhiskerRule: Tukey 1.5×IQR, MinMax, Percentile. Notched mode.
Outlier dots. Grouped mode (multiple columns side by side).

**Acceptance**: quartiles correct, whiskers correct, ≥4 tests.

**Dependencies**: none (day 1)

---

### T18 — ViolinSeries (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/ViolinSeries.h` / `.cpp`
- `tests/unit/test_violin.cpp`
- `tests/unit/test_violin_split.cpp`

KDE with Silverman bandwidth. Split-violin for paired comparison.
paint() draws the KDE shape as a filled polygon symmetric about
the center axis.

**Acceptance**: KDE shape matches known distribution, split works,
≥4 tests.

**Dependencies**: none (day 1)

---

### T19 — Statistical Commands (Backend, Size: S)

Bundled commands for Histogram, BoxPlot, Violin property changes.
Pattern from ChangeLineStyleCommand.

**Dependencies**: T16, T17, T18

---

### T20 — Statistical Dialogs (Frontend, Size: M)

HistogramPropertyDialog, BoxPlotPropertyDialog, ViolinPropertyDialog.
Each follows LinePropertyDialog pattern.

**Dependencies**: T19

---

### T21 — QA Statistical Tests (QA, Size: M)

6 tests per spec.

**Dependencies**: T16-T20

---

### M7.4 — Gate

All stat plots render from TabularBundle. 410 existing tests
unchanged. Target: ≥480 total.

---

## Closing

### T22 — Integration (Integration, Size: S)

Sub-phase merges, vphase-7 tag.

---

### T23 — Docs Closing (REVIEW AND STATUS IN SAME COMMIT)

**HARD RULE (verbatim, Phase 3b/4/5/6 lesson)**: docs/reviews/
phase-7-review.md MUST be WRITTEN AND COMMITTED IN THE SAME COMMIT
as the closing .lumen-ops/STATUS.md entry. One commit, one `git add`,
one `git commit`. The coordinator MUST enforce this.

**Files in ONE commit**:
- `docs/reviews/phase-7-review.md`
- `.lumen-ops/STATUS.md`
- `README.md`
- `src/lumen/core/reactive/CLAUDE.md`
- `src/lumen/plot/CLAUDE.md` (update for new types)

After: `git tag vphase-7 && git push origin main --tags`.

**Dependencies**: M7.4 gate

---

## Parallel Execution Schedule

```
Sub-phase 7.1 (~6h):
  Backend: T1 (reactive core) → T2 (Dataset integration)
  Frontend: T3 (ReactivityModeWidget) after T2
  QA: T4

Sub-phase 7.2 (~10h):
  Backend: T5 (Colormap, day 1) + T6 (Heatmap) + T7 (CPU renderer)
  Frontend: T8 (GPU layer, day 1) + T9 (GPU renderer) + T10 (dialog)
  QA: T11

Sub-phase 7.3 (~6h):
  Backend: T12 (CONREC, day 1) + T13 (ContourPlot)
  Frontend: T14 (dialog)
  QA: T15

Sub-phase 7.4 (~8h):
  Backend: T16+T17+T18 (parallel, day 1) → T19 (commands)
  Frontend: T20 (dialogs)
  QA: T21

Closing (~30min):
  T22 (merge, tag) + T23 (review+STATUS SAME commit)
```

**Total wall time**: ~30-36 hours across sub-phases.

## Risks

- OpenGL context fails → CPU fallback (documented in T8)
- CONREC edge cases → dedicated test fixtures (T12)
- Static snapshot doubles memory → MemoryManager integration (T1)
- Bidirectional feedback loops → generation counter (T1)
- KDE bandwidth heuristics → Silverman default + override (T18)
