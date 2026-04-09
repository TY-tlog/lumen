# ADR-019: HitTester extracted into plot/ layer

## Status
Accepted (Phase 3a); resolves ADR-017's deferred upgrade path.
**Extended in Phase 3b.** hitNonSeriesElement() added for
axis/title/legend region detection. hitTestPoint() added for
nearest-sample crosshair. Foundation for ADR-017 resolution;
ADR-017 itself fully resolved in Phase 3b via
HitTester::hitTestPoint extension. Original hitTest() interface
unchanged.

## Context
ADR-017 documented that the Phase 2 crosshair shows cursor position
in data space, not the nearest data point, because hit-testing was
not implemented. The ADR specified a Phase 4 upgrade trigger: "user
needs exact measurement of data values (e.g., spike peak voltage)."

Phase 3a needs hit-testing for a different reason: double-clicking
a line series to open the property editor requires knowing which
series the cursor is near. This is the same spatial query that
nearest-point crosshair needs, just at a coarser level (series-
level hit, not point-level hit).

## Decision
Extract HitTester as a standalone class in `src/lumen/plot/`:

```cpp
namespace lumen::plot {
class HitTester {
public:
    struct HitResult {
        int seriesIndex = -1;
        double pixelDistance = 0.0;
    };

    static std::optional<HitResult> hitTest(
        const PlotScene& scene,
        const CoordinateMapper& mapper,
        QPointF pixelPos,
        double tolerancePx = 5.0);
};
}
```

Algorithm: for each visible series, call buildPolylines(), map
each segment's endpoints to pixel coordinates via CoordinateMapper,
compute perpendicular distance from the query point to each
segment. Return the series with the minimum distance if within
tolerance.

HitTester lives in plot/ (not ui/) because it depends only on
PlotScene, CoordinateMapper, and LineSeries — all plot/ types.
This keeps it testable without QWidget dependencies.

### Relationship to ADR-017
This resolves ADR-017's foundation requirement. Phase 3a uses
HitTester for series-level hit detection (double-click → which
series?). Phase 4 extends it with point-level hit detection
(hover → which data point?) for the nearest-point crosshair
upgrade described in ADR-017.

The extension path:
1. Phase 3a: `hitTest()` returns series index (implemented now).
2. Phase 4: add `hitTestPoint()` returning series index + row
   index + exact (x, y) data values. Uses binary search on X
   column (sorted) for O(log n) per series.

## Consequences
- + Series-level hit detection for double-click editing
- + Foundation for Phase 4 nearest-point crosshair
- + Testable in isolation (no UI dependency)
- + O(n) per series: 7,000 segments in <1ms (acceptable)
- - Builds polylines on every hit test call. For Phase 4 with
  frequent mouse-move events, cache polylines in pixel space.
  Not needed for Phase 3a (double-click only).
- - Does not handle overlapping series well (returns nearest,
  not a disambiguation menu). Acceptable for Phase 3a.

## Alternatives considered
- **Inline hit-testing in PlotCanvas**: rejected; violates layering
  (ui/ should not compute geometric distances on plot data). Also
  not unit-testable without a widget.
- **GPU-based picking (render each series in a unique color to an
  offscreen buffer, read pixel)**: elegant and O(1), but requires
  OpenGL or a second QPainter render pass. Overkill for Phase 3a
  data sizes. Consider for Phase 6+ if data exceeds 100K points.
- **Spatial index (R-tree, grid)**: O(log n) query time, but
  O(n log n) build time and significant implementation complexity.
  Not worth it until data size exceeds the brute-force threshold
  (~50K segments, where brute-force exceeds 16ms).
