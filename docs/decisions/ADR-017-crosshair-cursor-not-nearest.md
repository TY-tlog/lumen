# ADR-017: Crosshair shows cursor position, not nearest data point

## Status
Accepted (Phase 2); upgrade to nearest-point snap targeted for
Phase 3b.

**To be resolved in Phase 3b** (see Task T6.5 in
docs/plans/phase-3b-plan.md). HitTester will gain a
hitTestPoint() method using binary search on sorted X columns;
PlotCanvas crosshair will call hitTestPoint() instead of
CoordinateMapper::pixelToData. When no sample is within pixel
tolerance, crosshair is hidden rather than showing interpolated
values.

Phase 3a built the HitTester foundation (ADR-019) for
series-level hit detection. Phase 3b extends it to point-level
detection, fully resolving this ADR.

## Context
The Phase 2 spec (D5) describes a crosshair that shows "coordinate
tooltip at cursor position (data coordinates, formatted)." This
could mean either:

(a) Show the data-space coordinates at the cursor's pixel position
    (inverse mapping via CoordinateMapper). This is what the user
    sees everywhere on the plot area, regardless of whether a data
    point is nearby.

(b) Snap to the nearest data point on the closest series and show
    that point's exact values. This is what MATLAB's data cursor
    and matplotlib's mplcursors do.

Option (b) requires a hit-testing system: for each mouse position,
search all visible series to find the nearest data point within a
pixel tolerance. This involves spatial indexing or brute-force
search over all points.

## Decision
Phase 2 implements option (a): cursor position in data space.

The crosshair draws vertical and horizontal lines at the cursor
pixel position and displays a tooltip with data coordinates
formatted to 4 significant digits. No hit-testing, no series
awareness, no snap.

## Consequences
- + Instant: no search needed, just a single pixelToData() call
- + Always available: works even when cursor is far from data
- + Simple to implement: ~30 lines in PlotCanvas::drawCrosshair()
- + Useful for rough coordinate reading (e.g., "the spike peak
  is around time 150 ms, voltage +20 mV")
- - Not useful for exact measurement: the displayed coordinates
  are the cursor position, not a data point value. For
  electrophysiology analysis (spike peak detection, threshold
  measurement), the user needs exact data point values.
- - Does not highlight which series/point the user is near

## Upgrade trigger (Phase 4)
Implement nearest-point snap when:
- The user needs exact measurement of data values (e.g., spike
  peak amplitude, inter-spike interval at cursor)
- The property inspector (Phase 5) needs to know which element
  the user is pointing at
- HitTester class is implemented as part of ADR-016 extraction

The upgrade path:
1. Implement HitTester: for each series, binary search on X
   column (sorted) to find nearest point within pixel tolerance.
2. If a point is found within tolerance: snap crosshair to that
   point, display exact (x, y) from the Column data, highlight
   the point with a marker.
3. If no point nearby: fall back to current behavior (cursor
   position in data space).
4. Add a status bar readout: "Series: voltage_mV, Point 1523:
   (305.0 ms, -38.23 mV)"

## Alternatives considered
- **Implement nearest-point snap now**: rejected; requires
  HitTester, binary search, tolerance calculation, marker
  rendering, series disambiguation — significant work for a
  feature that isn't blocking the Phase 2 demo. The cursor-
  position crosshair is sufficient for "this plot works and
  the data looks right."
- **No crosshair at all**: rejected; even rough coordinate
  reading is valuable for verifying the plot renders correct
  data ranges.
- **Vertical line only (no horizontal)**: some tools do this
  to reduce clutter. Rejected; both lines are useful for
  scientific data where both X and Y values matter.
