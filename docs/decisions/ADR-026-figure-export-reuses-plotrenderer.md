# ADR-026: Figure export reuses PlotRenderer

## Status
Accepted (Phase 4)

## Context
Phase 4.2 adds figure export to PNG, SVG, and PDF. The exported
figure must look identical to what the user sees on screen. There
are two ways to produce the export: reuse the existing
PlotRenderer::render() with a different QPainter target, or write
a separate export renderer.

## Decision
FigureExporter constructs the appropriate QPaintDevice (QImage for
PNG, QSvgGenerator for SVG, QPdfWriter for PDF), creates a QPainter
on it, and calls the existing `PlotRenderer::render(painter, scene,
targetSize)`.

This means there is exactly ONE rendering code path for both
screen and export. The only differences are:
- Target device (widget vs file-backed)
- Size (widget size vs export size, which may differ)
- DPI (screen DPI vs export DPI, affecting font metrics)
- Background (white vs transparent, controlled by whether
  PlotRenderer fills the background)

For transparent background in PNG: FigureExporter fills the QImage
with Qt::transparent before calling render. PlotRenderer's
background fill overwrites this with white normally, so
FigureExporter must either (a) skip the background fill by passing
a flag, or (b) post-process to replace white with transparent.
Option (a) is cleaner: add a `bool skipBackground` parameter to
PlotRenderer::render or use a separate fill step.

## Consequences
- + Single rendering code path: no drift between screen and export
- + Existing tests for PlotRenderer indirectly validate export
- + Simple implementation: FigureExporter is ~100 lines
- + Qt handles font embedding in SVG and PDF automatically
- - Font metrics differ between screen and export at different
  DPIs. This may cause minor layout differences (tick label
  positions, margin widths). Acceptable for Phase 4; can be
  addressed by passing explicit DPI to computeMargins in a future
  phase.
- - QSvgGenerator and QPdfWriter require additional Qt modules
  (Qt6::Svg, Qt6::Gui's QPdfWriter). Verify availability.

## Alternatives considered
- **Separate ExportRenderer**: a dedicated class that reads
  PlotScene and produces file output independently. Rejected:
  two rendering code paths inevitably diverge (one gets a fix,
  the other doesn't). The whole point of using QPainter is that
  it abstracts the target device.
- **QWidget::grab() to pixmap**: capture the PlotCanvas widget as
  a QPixmap, save to file. Rejected: limited to the current
  widget size (no export at higher resolution), raster only (no
  SVG/PDF), and captures UI chrome (dock borders, etc.) if not
  carefully clipped.
