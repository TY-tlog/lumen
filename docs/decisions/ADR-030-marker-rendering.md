# ADR-030: Marker rendering via QPainter primitives, pixel sizing

## Status
Accepted (Phase 5)

## Context
ScatterSeries renders a marker at each data point. The markers
must be visually clear, consistent across screen and export (PNG/
SVG/PDF), and performant for up to ~10K points.

Two decisions: how to render markers (QPainter primitives vs SVG
icons) and how to size them (pixel units vs data units).

## Decision
Render 6 fixed shapes via QPainter primitives. Marker size is in
pixels, not data units.

Shapes: Circle, Square, Triangle (up), Diamond, Plus (+), Cross (x).

Each shape is drawn with 2-4 QPainter calls (e.g., drawEllipse for
circle, drawPolygon for triangle). No external SVG files, no QIcon
rendering.

Size: specified in pixels (e.g., 6px radius). The marker appears
the same size regardless of zoom level. This matches MATLAB,
matplotlib, and Prism behavior.

## Consequences
- + Fast: QPainter primitives are hardware-accelerated
- + Consistent: same pixel size on screen and in export (at the
  export's DPI)
- + No external assets: markers are code, not files
- + Zoom-independent: markers don't shrink to invisible when
  zooming out
- - Limited to 6 shapes (sufficient for scientific plots; more
  can be added later without architectural change)
- - Pixel sizing means markers don't scale with data range.
  Acceptable: this is the convention in all major plotting tools.

## Alternatives considered
- **SVG or QIcon markers**: load SVG files for each shape, render
  via QPainter::drawPixmap or QSvgRenderer. Rejected: DPI
  inconsistency between screen and export (SVG renders at native
  DPI, which may differ from QPainter's target DPI), and
  unnecessary file I/O.
- **Data-unit sizing**: marker radius in data units (e.g., 0.5
  mV). Rejected: markers shrink when zooming out and grow when
  zooming in, which is visually jarring and differs from user
  expectations based on MATLAB/matplotlib.
