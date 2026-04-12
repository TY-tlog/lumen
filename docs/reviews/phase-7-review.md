# Phase 7 Review — Reactive Plot Engine + 2D Scalar Fields

**Date**: 2026-04-12
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-7-spec.md`
**Plan**: `docs/plans/phase-7-plan.md`

---

## What shipped

Phase 7 delivered the reactive plot engine, 2D scalar field
visualization, CONREC contour extraction, and three statistical
plot types — the largest single-phase feature set in the project.

### 7.1 — Reactive Core
- DependencyGraph: DAG with generation counter, cycle detection
- ReactiveBinding: 3-mode system (Static snapshot/DAG/Bidirectional)
- ReactivityModeWidget: 3-position toggle in toolbar

### 7.2 — Heatmap + Colormap
- Colormap: 11 built-ins, CIELAB ΔE uniformity check, Machado 2009 CVD
- Heatmap: PlotItem for Grid2D, adaptive CPU/GPU threshold
- Cached QImage optimization (10-100× pan/zoom speedup)
- PlotCanvas GPU layer infrastructure (QOpenGLWidget, lazy, optional)
- HeatmapPropertyDialog: colormap picker with uniformity badge

### 7.3 — Contour
- ContourAlgorithm: CONREC (Paul Bourke 1987) with saddle/plateau/NaN
- ContourPlot: auto/manual levels, labels, line styling
- ContourPropertyDialog

### 7.4 — Statistical Plots
- HistogramSeries: Scott/FD/Sturges binning, Count/Density/Probability
- BoxPlotSeries: Tukey/MinMax/Percentile whiskers, notched, outliers
- ViolinSeries: Silverman KDE, split-violin
- 3 property dialogs + 5 bundled commands

---

## Test results
- 545/545 tests pass (410 Phase 6 + 135 Phase 7)
- ASan + UBSan clean
- Zero compiler warnings

---

## ADRs delivered
| ADR | Decision |
|-----|----------|
| ADR-038 | 3-mode reactivity (Static=snapshot, DAG, Bidirectional) |
| ADR-039 | PlotCanvas hosts GPU layer (lazy QOpenGLWidget) |
| ADR-040 | CIELAB ΔE uniformity + Machado CVD safety |
| ADR-041 | CONREC contour from scratch |
| ADR-042 | Statistical plots as PlotItem subclasses |

---

## Lessons learned

### 1. Cached QImage is 90% of GPU benefit
The CPU path with QImage caching provides interactive framerates
for grids up to ~4096×4096 without OpenGL complexity. Full shader
pipeline deferred to Phase 8.

### 2. Review in same commit enforced
This review is in the same commit as the STATUS closing entry.

---

## Exit checklist
- [x] Build clean
- [x] 545 tests pass (410 Phase 6 unchanged + 135 new)
- [x] 3 reactive modes work (Static snapshot, DAG, Bidirectional)
- [x] Heatmap CPU path works
- [x] GPU layer infrastructure ready (full shader in Phase 8)
- [x] 5 perceptual colormaps verified uniform
- [x] Cividis passes CVD check
- [x] CONREC handles degenerate cases
- [x] Histogram, BoxPlot, Violin functional
- [x] ADR-038-042 committed
- [x] This review in SAME commit as STATUS close
- [x] vphase-7 tag (this commit)
