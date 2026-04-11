# Phase 7 — Reactive Plot Engine + 2D Scalar Fields

## Goal

Add reactive plot updates and 2D scalar field visualization on
top of Phase 6's Dataset foundation. Three new PlotItem types
(Heatmap, Contour, HistogramSeries + BoxPlot + ViolinPlot) turn
Phase 6's Grid2D and TabularBundle into first-class visualizations.
A novel 3-mode reactivity system (Static / DAG / Bidirectional)
lets the user pick the data flow model appropriate for each
analysis task — a capability no existing plotting tool offers as
a first-class choice.

This phase delivers the first payoff of the Phase 6 foundation
and advances all three north-star values: completeness (new
plot types), convenience (reactive updates), originality
(selectable reactive modes, perceptual colormaps, CONREC-grade
contours).

## Design decisions (approved)

1. **Reactivity**: 3-mode system, user-selectable per plot
   (Static / DAG / Bidirectional)
2. **Heatmap rendering**: adaptive CPU/GPU with 1024×1024
   threshold
3. **Colormaps**: perceptually uniform guarantee + colorblind
   safety metadata
4. **Contour algorithm**: CONREC-grade
5. **Statistical plots**: included in this phase

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: four sub-phases

- **7.1** Reactive core (3-mode system, dependency graph)
- **7.2** Heatmap + Colormap system
- **7.3** Contour (CONREC) + adaptive rendering
- **7.4** Statistical plots (Histogram, BoxPlot, ViolinPlot)

Each sub-phase ends with a verification gate.

## Phase 7.1 — Reactive Core

### core/reactive/ReactiveMode.h
namespace lumen::reactive {
enum class Mode {
Static,        // (a) No auto-updates. User re-renders manually.
DAG,           // (b) Dataset change propagates forward through
//     derived datasets to plot.
Bidirectional, // (c) Plot edits push back to Dataset.
};
}

### core/reactive/DependencyGraph.{h,cpp}

DAG of Datasets and derived values:
class DependencyGraph : public QObject {
Q_OBJECT
public:
void addNode(Dataset* ds);
void addDerivation(Dataset* source, Dataset* derived,
DerivationFn fn);
void propagate(Dataset* changed);  // cascade updates
std::vector<Dataset*> downstream(Dataset* ds) const;
signals:
void nodeUpdated(Dataset* ds);
};

### core/reactive/ReactiveBinding.{h,cpp}

Per-plot reactive binding:
class ReactiveBinding : public QObject {
Q_OBJECT
public:
ReactiveBinding(PlotItem* plot, Mode mode);
void setMode(Mode m);
Mode mode() const;

void bindDataset(Dataset* ds);
// Static: does nothing on ds->changed()
// DAG: listens and schedules plot repaint via DependencyGraph
// Bidirectional: DAG + installs plot-edit interceptor that
//                writes back to ds
signals:
void modeChanged(Mode);
};

### ui/ReactivityModeWidget.{h,cpp}

Small widget in plot property dialog (extends Phase 3a Line/
Scatter/Bar dialogs) showing current mode with a 3-position
toggle. Tooltip explains each mode.

### Tests 7.1

- test_reactive_mode.cpp: mode transitions
- test_dependency_graph.cpp: add, propagate, downstream lookup,
  cycle detection
- test_static_mode.cpp: ds->changed() does NOT repaint plot
- test_dag_mode.cpp: ds->changed() triggers plot repaint within
  one event loop
- test_bidirectional_mode.cpp: plot edit writes back to ds
- test_mode_switching.cpp: switching mode at runtime works
  without data loss

## Phase 7.2 — Heatmap and Colormap

### plot/Heatmap.{h,cpp} (new PlotItem subclass)
class Heatmap : public PlotItem {
public:
Heatmap(Grid2D* grid, Colormap cmap);
Type type() const override { return Type::Heatmap; }
void paint(QPainter*, const CoordinateMapper&,
           const QRectF&) const override;
std::optional<HitResult> hitTestPoint(
    QPoint, const CoordinateMapper&, double) const override;
QRectF dataBounds() const override;

void setColormap(Colormap);
void setValueRange(double min, double max);  // manual override
void setInterpolation(bool bilinear);
void setOpacity(double);

// Adaptive rendering: < 1024x1024 CPU path, >= 1024x1024 GPU
RenderPath activeRenderPath() const;
signals:
void changed();
};

### plot/Colormap.{h,cpp}
class Colormap {
public:
enum class Builtin {
Viridis, Plasma, Inferno, Magma, Turbo,   // perceptual
Cividis,                                    // colorblind-optimal
Gray, Hot, Cool,                            // classic
RedBlue, BrownTeal                          // diverging
};
static Colormap builtin(Builtin id);
static Colormap fromFile(const QString& path);  // user JSON
static Colormap fromControlPoints(
    const std::vector<std::pair<double, QColor>>& stops);

QColor sample(double t) const;  // t in [0, 1]

bool isPerceptuallyUniform() const;  // computed at load
bool isColorblindSafe() const;        // metadata + heuristic

QString name() const;
QJsonObject toJson() const;
void saveToFile(const QString& path) const;
};

Perceptual uniformity check computes ΔE in CIELAB between
adjacent samples; if variance < threshold, marked uniform.
Colorblind safety uses CVD simulation (deuteranopia,
protanopia, tritanopia) and checks distinguishability.

### plot/HeatmapRenderer.{h,cpp}

Two paths:

- **CPU path**: QImage, loop over grid cells, sample colormap,
  write pixel. Used when grid <= 1024×1024.
- **GPU path**: QOpenGLWidget child embedded in PlotCanvas.
  Grid uploaded as GL_R32F texture. Fragment shader samples
  colormap LUT. Used when grid > 1024×1024.

Architect decides the exact seam: whether Heatmap owns the
OpenGL widget, or PlotCanvas gains a "gpu layer" that Heatmap
requests.

### Tests 7.2

- test_colormap_builtin.cpp: each built-in samples correctly
- test_colormap_perceptual.cpp: viridis, plasma, inferno,
  magma, cividis all pass uniformity check
- test_colormap_colorblind.cpp: cividis passes CVD distinguish
  test
- test_colormap_json_roundtrip.cpp: save, load, identical
- test_heatmap_cpu_path.cpp: 256×256 grid renders to QImage
  with correct colors
- test_heatmap_gpu_path.cpp: 2048×2048 grid engages GPU path
  (skip if no GL context)
- test_heatmap_adaptive_threshold.cpp: 1024×1024 threshold
  exact boundary
- test_heatmap_value_range_override.cpp: manual min/max honored

## Phase 7.3 — Contour Plot

### plot/ContourPlot.{h,cpp}
class ContourPlot : public PlotItem {
public:
ContourPlot(Grid2D* grid);
void setLevels(std::vector<double> levels);  // manual
void setAutoLevels(int count);                // compute from data range
void setLabelsVisible(bool);
void setLineColor(QColor);  // or per-level via colormap
void setLineWidth(double);

// ... PlotItem overrides
};

### plot/ContourAlgorithm.{h,cpp}

CONREC-grade contour extraction:
struct ContourSegment {
QPointF a;
QPointF b;
double level;
};
class ContourAlgorithm {
public:
static std::vector<ContourSegment> extract(
const Grid2D& grid,
const std::vector<double>& levels);
};

Implementation: Paul Bourke's CONREC or equivalent triangular
subdivision marching squares. Handles degenerate cases (value
exactly on level), produces consistent contour topology.

### Tests 7.3

- test_contour_algorithm.cpp: known analytic input (gaussian,
  saddle, monotonic) produces expected level sets
- test_contour_degenerate.cpp: value exactly on level handled
- test_contour_auto_levels.cpp: 10 levels from [0, 1] data
- test_contour_with_heatmap.cpp: contour overlays heatmap
  correctly (same Grid2D, stacked rendering)

## Phase 7.4 — Statistical Plots

Three new PlotItem subclasses operating on TabularBundle
columns (not Grid2D):

### plot/HistogramSeries.{h,cpp}
class HistogramSeries : public PlotItem {
public:
HistogramSeries(Dataset* column);
void setBinCount(int n);             // manual
void setAutoBinning(BinRule rule);   // Scott, Freedman-Diaconis, Sturges
void setNormalization(Norm n);        // Count, Density, Probability
void setFillColor(QColor);
// ...
};

### plot/BoxPlotSeries.{h,cpp}
class BoxPlotSeries : public PlotItem {
public:
BoxPlotSeries(Dataset* column);      // single
BoxPlotSeries(std::vector<Dataset*> columns);  // grouped
void setWhiskerRule(WhiskerRule r);  // Tukey 1.5*IQR, min-max, percentile
void setNotched(bool);                // 95% CI notches
void setOutliersVisible(bool);
// ...
};

### plot/ViolinSeries.{h,cpp}
class ViolinSeries : public PlotItem {
public:
ViolinSeries(Dataset* column);       // single
ViolinSeries(std::vector<Dataset*> columns);  // grouped
void setKdeBandwidth(double);
void setKdeBandwidthAuto(bool);       // Silverman's rule
void setSplit(bool);                  // split-violin for pairs
// ...
};

### Tests 7.4

- test_histogram.cpp: binning rules, normalization modes
- test_histogram_auto_bins.cpp: Scott, FD, Sturges on known data
- test_boxplot.cpp: quartile computation, whisker rules
- test_boxplot_grouped.cpp: multi-column layout
- test_violin.cpp: KDE with Silverman bandwidth
- test_violin_split.cpp: split-violin pair

## ADRs

- ADR-038 Three-mode reactivity system. Alternatives: always-
  reactive (rejected, no escape hatch), always-static (rejected,
  loses Observable-like updates), mode per dataset not per plot
  (rejected, plot is the user's mental unit)
- ADR-039 Adaptive heatmap rendering with 1024×1024 threshold.
  Alternatives: always-CPU (rejected, large grids slow),
  always-GPU (rejected, overhead for small, driver complexity)
- ADR-040 Perceptual uniformity requirement for default
  colormaps. Alternatives: any colormap allowed (rejected,
  undermines scientific validity), strict whitelist only
  (rejected, users want custom)
- ADR-041 CONREC-grade contour algorithm. Alternatives: basic
  marching squares (rejected, degenerate handling), library
  dependency (rejected, add-weight)
- ADR-042 Statistical plots as PlotItem subclasses. Alternatives:
  separate StatPlot hierarchy (rejected, splits the type system)

## Architecture updates

docs/architecture.md Phase 7 section:
- New core/reactive/ submodule
- New plot/ types: Heatmap, ContourPlot, HistogramSeries,
  BoxPlotSeries, ViolinSeries
- Colormap system as plot/ infrastructure
- Adaptive rendering: CPU QPainter path and GPU QOpenGLWidget
  path coexist in PlotCanvas
- Reactive binding layer between Dataset and PlotItem

## Acceptance criteria

Manual flow:

7.1: Open a TabularBundle, add line plot, switch reactive mode
     in plot property dialog to DAG. Modify underlying Dataset
     programmatically (or via Phase 13 future hook). Plot
     updates automatically. Switch to Static. Modify again.
     Plot does not update until manual refresh.

7.2: Open Gaussian 2D sample (Phase 6 sample menu) → automatic
     heatmap with viridis colormap. Change to plasma via dialog.
     Verify perceptual uniformity badge shown. Load Mandelbrot
     sample → 2048×2048 → GPU path engaged (status bar hint).

7.3: On same Gaussian 2D, add contour overlay with 10 auto
     levels. Contours smooth, labeled. Verify CONREC handles
     saddle points.

7.4: Open a TabularBundle with a numeric column, add histogram
     → auto-binned. Switch to Freedman-Diaconis rule → bin count
     changes. Add box plot → quartiles visible. Add violin plot
     → KDE shape visible.

Regression:
- [ ] All 410 Phase 6 tests pass unchanged
- [ ] Phase 2-5 plot types (line, scatter, bar) still work
- [ ] Phase 3a/3b editing still works on all plot types
- [ ] Phase 4 workspace saves/loads new plot types
- [ ] Phase 4 export (PNG/SVG/PDF) renders new types

## Real-data exit criterion

Human loads a real 2D scientific dataset (e.g., microscopy
image via TIFF loader, FFT spectrogram via HDF5, or computed
gaussian from Python) and:

- [ ] Visualizes as heatmap with appropriate colormap
- [ ] Adds contour overlay
- [ ] Plots a 1D slice of the same data as line plot alongside
- [ ] Plots distribution of values as histogram
- [ ] Switches reactive mode and observes behavior matches
- [ ] Saves workspace, reopens, all restored
- [ ] Exports as PDF for publication

## Non-goals

- 3D plots (Phase 8)
- Publication-grade export enhancements (Phase 9)
- Style presets (Phase 10)
- Multi-plot dashboard (Phase 11)
- Annotations beyond existing text labels (Phase 12)
- Curve fitting, statistical tests (Phase 13)
- Custom contour labeling positioning (basic only)
- Log-scale colormaps (defer to Phase 9 or later)

## Risks

| Risk | Mitigation |
|---|---|
| OpenGL context creation fails on some systems | Fall back to CPU path with warning; document |
| CONREC edge cases (plateau, NaN) | Dedicated test fixtures for each |
| Perceptual uniformity check too strict, rejects legit maps | Threshold calibrated from known-good maps; configurable |
| Reactive bidirectional mode causes feedback loops | Explicit generation counter in DependencyGraph |
| Memory usage spikes with multiple large heatmaps | MemoryManager from Phase 6 tracks texture memory too |
| KDE bandwidth heuristics produce weird violins | Silverman default + user override |

## Task breakdown

### Architect (S)
Plan, ADR-038..042, architecture update, STATUS entry

### Backend (XL, across sub-phases)
- 7.1: reactive/ module
- 7.2: Heatmap, Colormap, CPU render path
- 7.3: ContourAlgorithm, ContourPlot
- 7.4: Histogram, BoxPlot, ViolinSeries + commands

### Frontend (L)
- 7.1: ReactivityModeWidget, plot dialog integration
- 7.2: GPU path (QOpenGLWidget integration), colormap picker UI
- 7.3: contour level editor
- 7.4: stat plot dialogs (HistogramPropertyDialog, etc.)

### QA (L)
Tests per sub-phase, regression 410-test gate, real-data
verification

### Integration (S)
Sub-phase merge windows, vphase-7 tag

### Docs (S)
README, module CLAUDE.md updates, phase-7-review.md IN SAME
COMMIT as closing STATUS entry

## Exit checklist

- [ ] Build clean
- [ ] All 410 Phase 6 tests pass unchanged
- [ ] New Phase 7 tests pass (target 410 → 520+)
- [ ] 3 reactive modes demonstrably work
- [ ] Heatmap CPU and GPU paths both work
- [ ] 5 built-in perceptual colormaps verified uniform
- [ ] Cividis passes colorblind-safe check
- [ ] CONREC handles degenerate cases
- [ ] Histogram, BoxPlot, Violin functional
- [ ] Real-data exit criterion passed
- [ ] ADR-038..042 committed
- [ ] phase-7-review.md committed IN SAME COMMIT as closing
      STATUS entry
- [ ] vphase-7 tag pushed
