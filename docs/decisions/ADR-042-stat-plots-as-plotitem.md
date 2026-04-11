# ADR-042: Statistical plots as PlotItem subclasses

## Status
Accepted (Phase 7)

## Context
Phase 7 adds three statistical plot types: Histogram, BoxPlot,
Violin. These operate on 1D data (TabularBundle columns) rather
than 2D grids, and compute derived values (bin counts, quartiles,
KDE) before rendering.

The question is whether these fit into the Phase 5 PlotItem
hierarchy or need a separate type system.

## Decision
Statistical plots are PlotItem subclasses, alongside LineSeries,
ScatterSeries, BarSeries, Heatmap, and ContourPlot. They implement
the same virtual interface (paint, dataBounds, type, isVisible,
name, primaryColor) and participate in all existing infrastructure:
PlotScene container, PlotRenderer iteration, HitTester dispatch,
CommandBus commands, workspace save/load, figure export.

Type enum extended:
```cpp
enum class Type {
    Line, Scatter, Bar,        // Phase 5
    Heatmap, Contour,          // Phase 7.2/7.3
    Histogram, BoxPlot, Violin // Phase 7.4
};
```

Each stat plot type computes its derived data (bins, quartiles, KDE)
in its constructor or on invalidate(), caches the result, and
renders from the cache in paint(). The computation is not in
PlotRenderer — each type is self-rendering per the PlotItem contract
from ADR-028.

## Consequences
- + Unified type system: all plot types share the same container,
  rendering, editing, saving, and export infrastructure
- + No special-casing in PlotRenderer, HitTester, or WorkspaceFile
- + User sees all types in the same column picker + type combo
- + CommandBus commands follow the same bundled pattern
- + Phase 8 (3D) adds more PlotItem subclasses naturally
- - Stat plots compute derived data inside paint() lifecycle, which
  is a different pattern than Line/Scatter/Bar (which just iterate
  raw data). Acceptable: the cache means computation happens once,
  rendering is fast.
- - dataBounds() for Histogram depends on computed bins, not raw
  data range. Acceptable: the bounds are still a QRectF.

## Alternatives considered
- **Separate StatPlot hierarchy**: a parallel base class for
  statistical plots with its own render/save/export paths.
  Rejected: splits the type system, doubles infrastructure code,
  and confuses users who expect all plots to behave the same way.
- **matplotlib-style axes methods** (ax.hist, ax.boxplot): function
  calls that produce plot elements. Rejected: Lumen uses a
  declarative object model (PlotItem), not an imperative API.
  Objects persist, can be edited, saved, and undone.
